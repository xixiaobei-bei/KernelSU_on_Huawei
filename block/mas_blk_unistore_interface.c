/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 * Description: mas block unistore interface for hmfs
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt) "[BLK-IO]" fmt

#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include <linux/module.h>
#include <linux/bio.h>
#include <linux/delay.h>
#include <linux/gfp.h>
#include <linux/hisi/powerkey_event.h>
#include <trace/events/block.h>
#include <linux/types.h>
#include <trace/iotrace.h>
#include "blk.h"
#include "dsm_block.h"

#ifdef CONFIG_MAS_UNISTORE_PRESERVE
int mas_blk_device_pwron_info_sync(struct block_device *bi_bdev,
	struct stor_dev_pwron_info *stor_info, unsigned int rescue_seg_size)
{
	int ret;
	unsigned int i;
	struct request_queue *q = bdev_get_queue(bi_bdev);
	struct blk_dev_lld *lld = mas_blk_get_lld(q);

	if (!blk_queue_query_unistore_enable(q))
		return -EFAULT;

	if (!lld || !lld->unistore_ops.dev_pwron_info_sync)
		return -EPERM;

	io_trace_unistore_count(UNISTORE_PON_SYNC_SEND_CNT, 1);

	ret = lld->unistore_ops.dev_pwron_info_sync(q, stor_info, rescue_seg_size);
	if (ret) {
		io_trace_unistore_count(UNISTORE_PON_SYNC_FAIL_CNT, 1);
		return ret;
	}

	for (i = 0; i < STREAM_NUM; i++) {
		if (stor_info->dev_stream_addr[i] != 0)
			stor_info->dev_stream_addr[i] =
				stor_info->dev_stream_addr[i] -
				(unsigned int)(bi_bdev->bd_part->start_sect >> SECTION_SECTOR);
	}
	for (i = 0; i < stor_info->rescue_seg_cnt; i++)
		stor_info->rescue_seg[i] =
			(stor_info->rescue_seg[i] << SECTOR_BYTE) -
			(unsigned int)(bi_bdev->bd_part->start_sect >> SECTION_SECTOR);

	for (i = 0; i < DATA_MOVE_STREAM_NUM; i++) {
		if (stor_info->dm_stream_addr[i] != 0)
			stor_info->dm_stream_addr[i] =
				stor_info->dm_stream_addr[i] -
				(unsigned int)(bi_bdev->bd_part->start_sect >> SECTION_SECTOR);
	}

	return ret;
}

int mas_blk_stream_oob_info_fetch(struct block_device *bi_bdev,
				struct stor_dev_stream_info stream_info,
				unsigned int oob_entry_cnt,
				struct stor_dev_stream_oob_info *oob_info)
{
	struct request_queue *q = bdev_get_queue(bi_bdev);
	struct blk_dev_lld *lld = mas_blk_get_lld(q);
	int ret;

	if (!blk_queue_query_unistore_enable(q))
		return -EFAULT;

	if (!lld || !lld->unistore_ops.dev_stream_oob_info_fetch)
		return -EPERM;

	io_trace_unistore_count(UNISTORE_GET_STREAM_OOB_SEND_CNT, 1);

	stream_info.stream_start_lba += (
		bi_bdev->bd_part->start_sect >> SECTION_SECTOR);

	ret = lld->unistore_ops.dev_stream_oob_info_fetch(
		q, stream_info, oob_entry_cnt, oob_info);

	if (ret)
		io_trace_unistore_count(UNISTORE_GET_STREAM_OOB_FAIL_CNT, 1);

	return ret;
}

int mas_blk_device_reset_ftl(struct block_device *bi_bdev)
{
	struct request_queue *q = bdev_get_queue(bi_bdev);
	struct blk_dev_lld *lld = mas_blk_get_lld(q);
	struct stor_dev_reset_ftl reset_ftl_info = {0};
	int ret;

	if (!blk_queue_query_unistore_enable(q))
		return -EFAULT;

	if (!lld || !lld->unistore_ops.dev_reset_ftl)
		return -EPERM;

	ret = lld->unistore_ops.dev_reset_ftl(q, &reset_ftl_info);

	if (ret)
		io_trace_unistore_count(UNISTORE_RESET_FTL_FAIL_CNT, 1);

	return ret;
}

int mas_blk_device_close_section(struct block_device *bi_bdev,
	struct stor_dev_reset_ftl *reset_ftl_info)
{
	struct request_queue *q = bdev_get_queue(bi_bdev);
	struct blk_dev_lld *lld = mas_blk_get_lld(q);

	if (!blk_queue_query_unistore_enable(q))
		return -EFAULT;

	if (!lld || !lld->unistore_ops.dev_reset_ftl)
		return -EPERM;

	return lld->unistore_ops.dev_reset_ftl(q, reset_ftl_info);
}

int mas_blk_device_read_section(
	struct block_device *bi_bdev, unsigned int *section_size)
{
	struct request_queue *q = bdev_get_queue(bi_bdev);
	struct blk_dev_lld *lld = mas_blk_get_lld(q);

	if (!blk_queue_query_unistore_enable(q))
		return -EFAULT;

	if (!lld || !lld->unistore_ops.dev_read_section)
		return -EPERM;

	return lld->unistore_ops.dev_read_section(q, section_size);
}

int mas_blk_device_config_mapping_partition(
	struct block_device *bi_bdev,
	struct stor_dev_mapping_partition *mapping_info)
{
	struct request_queue *q = bdev_get_queue(bi_bdev);
	struct blk_dev_lld *lld = mas_blk_get_lld(q);

	if (!blk_queue_query_unistore_enable(q))
		return -EFAULT;

	if (!lld || !lld->unistore_ops.dev_config_mapping_partition)
		return -EPERM;

	return lld->unistore_ops.dev_config_mapping_partition(q, mapping_info);
}

int mas_blk_device_read_mapping_partition(
	struct block_device *bi_bdev,
	struct stor_dev_mapping_partition *mapping_info)
{
	struct request_queue *q = bdev_get_queue(bi_bdev);
	struct blk_dev_lld *lld = mas_blk_get_lld(q);

	if (!blk_queue_query_unistore_enable(q))
		return -EFAULT;

	if (!lld || !lld->unistore_ops.dev_read_mapping_partition)
		return -EPERM;

	return lld->unistore_ops.dev_read_mapping_partition(q, mapping_info);
}

int mas_blk_device_read_op_size(struct block_device *bi_bdev,
	int *op_size)
{
	int ret;
	struct request_queue *q = bdev_get_queue(bi_bdev);
	struct blk_dev_lld *lld = mas_blk_get_lld(q);

	if (!blk_queue_query_unistore_enable(q))
		return -EFAULT;

	if (!lld || !lld->unistore_ops.dev_read_op_size)
		return -EPERM;

	ret = lld->unistore_ops.dev_read_op_size(q, op_size);
	if (unlikely(ret))
		pr_err("%s, read op size failed, ret = %d\n", __func__, ret);

	return ret;
}

int mas_blk_fs_sync_done(struct block_device *bi_bdev)
{
	struct request_queue *q = bdev_get_queue(bi_bdev);
	struct blk_dev_lld *lld = mas_blk_get_lld(q);
	int ret;

	if (!blk_queue_query_unistore_enable(q))
		return -EFAULT;

	if (!lld || !lld->unistore_ops.dev_fs_sync_done)
		return -EPERM;

	io_trace_unistore_count(UNISTORE_FS_SYNC_DONE_SEND_CNT, 1);

	ret = lld->unistore_ops.dev_fs_sync_done(q);

	if (ret)
		io_trace_unistore_count(UNISTORE_FS_SYNC_DONE_FAIL_CNT, 1);

	return ret;
}

#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
int mas_blk_rescue_block_inject_data(struct block_device *bi_bdev, sector_t sect)
{
	struct request_queue *q = bdev_get_queue(bi_bdev);
	struct blk_dev_lld *lld = mas_blk_get_lld(q);
	unsigned int lba = sect + (bi_bdev->bd_part->start_sect >> SECTION_SECTOR);

	if (!blk_queue_query_unistore_enable(q))
		return -EFAULT;

	if (!lld || !lld->unistore_ops.dev_rescue_block_inject_data)
		return -EPERM;

	return lld->unistore_ops.dev_rescue_block_inject_data(q, lba);
}

int mas_blk_bad_block_error_inject(struct block_device *bi_bdev,
	unsigned char bad_slc_cnt, unsigned char bad_tlc_cnt)
{
	struct request_queue *q = bdev_get_queue(bi_bdev);
	struct blk_dev_lld *lld = mas_blk_get_lld(q);

	if (!blk_queue_query_unistore_enable(q))
		return -EFAULT;

	if (!lld || !lld->unistore_ops.dev_bad_block_err_inject)
		return -EPERM;

	return lld->unistore_ops.dev_bad_block_err_inject(q, bad_slc_cnt, bad_tlc_cnt);
}
#endif

int mas_blk_data_move(struct block_device *bi_bdev,
	struct stor_dev_data_move_info *data_move_info)
{
	unsigned int i;
	int ret;
	struct request_queue *q = bdev_get_queue(bi_bdev);
	struct blk_dev_lld *lld = mas_blk_get_lld(q);
	struct stor_dev_data_move_source_addr *source_addr = NULL;
	sector_t start_sect = bi_bdev->bd_part->start_sect >> SECTION_SECTOR;

	if (!blk_queue_query_unistore_enable(q))
		return -EFAULT;

	if (!data_move_info)
		return -EFAULT;
	if ((data_move_info->source_addr_num > DATA_MOVE_MAX_NUM) ||
			(data_move_info->source_inode_num > DATA_MOVE_MAX_NUM))
		return -EINVAL;

	io_trace_unistore_count(UNISTORE_DATA_MOVE_SEND_CNT, 1);

	data_move_info->dest_4k_lba += start_sect;
	for (i = 0; i < data_move_info->source_addr_num; i++) {
		source_addr = (data_move_info->source_addr) + i;
		source_addr->data_move_source_addr += start_sect;
	}

	if (!lld || !lld->unistore_ops.dev_data_move)
		return -EPERM;

	ret = lld->unistore_ops.dev_data_move(q, data_move_info);

	if (ret)
		io_trace_unistore_count(UNISTORE_DATA_MOVE_FAIL_CNT, 1);

	data_move_info->verify_info.next_to_be_verify_4k_lba -= start_sect;
	data_move_info->verify_info.next_available_write_4k_lba -= start_sect;

	return ret;
}

int mas_blk_slc_mode_configuration(struct block_device *bi_bdev, int *status)
{
	struct request_queue *q = bdev_get_queue(bi_bdev);
	struct blk_dev_lld *lld = mas_blk_get_lld(q);
	int ret;

	if (!blk_queue_query_unistore_enable(q))
		return -EFAULT;

	if (!lld || !lld->unistore_ops.dev_slc_mode_configuration)
		return -EPERM;

	io_trace_unistore_count(UNISTORE_SLC_MODE_SEND_CNT, 1);

	ret = lld->unistore_ops.dev_slc_mode_configuration(q, status);

	if (ret)
		io_trace_unistore_count(UNISTORE_SLC_MODE_FAIL_CNT, 1);

	return ret;
}

int mas_blk_sync_read_verify(struct block_device *bi_bdev,
	struct stor_dev_sync_read_verify_info *verify_info)
{
	int ret;
	struct request_queue *q = bdev_get_queue(bi_bdev);
	struct blk_dev_lld *lld = mas_blk_get_lld(q);
	sector_t start_sect = bi_bdev->bd_part->start_sect >> SECTION_SECTOR;

	if (!blk_queue_query_unistore_enable(q))
		return -EFAULT;

	verify_info->cp_verify_l4k += start_sect;
	verify_info->cp_open_l4k += start_sect;
	verify_info->cp_cache_l4k += start_sect;

	if (!lld || !lld->unistore_ops.dev_sync_read_verify)
		return -EPERM;

	io_trace_unistore_count(UNISTORE_SYNC_READ_VERIFY_SEND_CNT, 1);

	ret = lld->unistore_ops.dev_sync_read_verify(q, verify_info);

	if (ret)
		io_trace_unistore_count(UNISTORE_SYNC_READ_VERIFY_FAIL_CNT, 1);

	if (verify_info->verify_info.next_to_be_verify_4k_lba)
		verify_info->verify_info.next_to_be_verify_4k_lba -= start_sect;
	if (verify_info->verify_info.next_available_write_4k_lba)
		verify_info->verify_info.next_available_write_4k_lba -= start_sect;

	return ret;
}

int mas_blk_get_bad_block_info(struct block_device *bi_bdev,
	struct stor_dev_bad_block_info *bad_block_info)
{
	struct request_queue *q = bdev_get_queue(bi_bdev);
	struct blk_dev_lld *lld = mas_blk_get_lld(q);

	if (!blk_queue_query_unistore_enable(q))
		return -EFAULT;

	if (!lld || !lld->unistore_ops.dev_get_bad_block_info)
		return -EPERM;

	return lld->unistore_ops.dev_get_bad_block_info(q, bad_block_info);
}

int mas_blk_get_program_size(struct block_device *bi_bdev,
			struct stor_dev_program_size *program_size)
{
	int ret;
	struct request_queue *q = bdev_get_queue(bi_bdev);
	struct blk_dev_lld *lld = mas_blk_get_lld(q);

	if (!blk_queue_query_unistore_enable(q))
		return -EFAULT;

	if (!lld || !lld->unistore_ops.dev_get_program_size)
		return -EPERM;

	ret = lld->unistore_ops.dev_get_program_size(q, program_size);
	if (ret)
		pr_err("%s, ret=%d\n", __func__, ret);

	pr_err("%s, tlc_size=%x, slc_size=%x\n", __func__,
		program_size->tlc_program_size, program_size->slc_program_size);

	return ret;
}


static int mas_blk_set_streamid(struct hd_struct *p, struct blkdev_cmd *cmd)
{
	struct blkdev_set_stream stream;

	if (!cmd->cust_argp)
		return -EFAULT;

	if (copy_from_user(&stream, cmd->cust_argp, sizeof(struct blkdev_set_stream))) {
		pr_err("%s copy_from_user failed\n", __func__);
		return -EFAULT;
	}

	if (stream.stream_id > MAX_WRITE_STREAM_TYPE) {
		pr_err("%s, set wrong stream id:%u\n", __func__, stream.stream_id);
		return -EFAULT;
	}

	p->mas_hd.default_stream_id = stream.stream_id;
	return 0;
}

void mas_blk_fsync_barrier(struct block_device *bdev)
{
	unsigned long flags = 0;
	struct request_queue *q = NULL;
	struct blk_dev_lld *lld = NULL;

	if (unlikely(!bdev))
		return;

	q = bdev_get_queue(bdev);
	if (!blk_queue_query_unistore_enable(q))
		return;

	lld = mas_blk_get_lld(q);
	spin_lock_irqsave(&lld->fsync_ind_lock, flags);
	if (lld && !lld->fsync_ind)
		lld->fsync_ind = true;
	spin_unlock_irqrestore(&lld->fsync_ind_lock, flags);
}

struct stor_pwron_info_to_user {
	unsigned int dev_stream_addr[STREAM_NUM];
	unsigned int dm_stream_addr[DATA_MOVE_STREAM_NUM];
	unsigned char io_slc_mode_status;
};

/*lint -e644*/
static int mas_blk_get_open_ptr(struct block_device *bdev, struct blkdev_cmd *cmd)
{
	int err;
	int i;
	struct stor_dev_pwron_info stor_info;
	struct stor_pwron_info_to_user stor_info_to_user;

	struct request_queue *q = bdev_get_queue(bdev);

	if (!blk_queue_query_unistore_enable(q))
		return -EFAULT;

	stor_info.rescue_seg = vmalloc(sizeof(unsigned int) * MAX_RESCUE_SEG_CNT);
	if (unlikely(!stor_info.rescue_seg)) {
		pr_err("%s: alloc mem failed\n", __func__);
		return -ENOMEM;
	}

	err = mas_blk_device_pwron_info_sync(bdev, &stor_info,
			sizeof(unsigned int) * MAX_RESCUE_SEG_CNT);
	if (unlikely(err)) {
		pr_err("%s, blk_device_pwron_info_sync failed, err = %d\n", __func__, err);
		vfree(stor_info.rescue_seg);
		return err;
	}
	vfree(stor_info.rescue_seg);

	for (i = 0; i < STREAM_NUM; i++) {
		stor_info_to_user.dev_stream_addr[i] = stor_info.dev_stream_addr[i];
		pr_err("%s, dev_stream_addr[%d] = 0x%x\n",
				__func__, i, stor_info_to_user.dev_stream_addr[i]);
	}

	for (i = 0; i < DATA_MOVE_STREAM_NUM; i++) {
		stor_info_to_user.dm_stream_addr[i] = stor_info.dm_stream_addr[i];
		pr_err("%s, dm_stream_addr[%d] = 0x%x\n",
				__func__, i, stor_info_to_user.dm_stream_addr[i]);
	}

	stor_info_to_user.io_slc_mode_status = stor_info.io_slc_mode_status;
	pr_err("%s, io_slc_mode_status = 0x%x\n",
				__func__, stor_info_to_user.io_slc_mode_status);

	if (!cmd->cust_argp)
		return -EFAULT;

	err = copy_to_user(cmd->cust_argp, &stor_info_to_user,
				sizeof(struct stor_pwron_info_to_user));
	if (unlikely(err))
		pr_err("%s, copy_to_user failed, err = %d\n", __func__, err);

	return err;
}
/*lint -e644*/

static int mas_blk_get_segments_per_section(
	struct block_device *bdev, struct blkdev_cmd *cmd)
{
	int err;
	unsigned int section_size = 0;
	unsigned int segments_per_section;

	err = mas_blk_device_read_section(bdev, &section_size);
	if (err) {
		pr_err("%s, blk_device_read_section failed, err = %d\n",
							__func__, err);
		return err;
	}

	segments_per_section = section_size >> SECTOR_BYTE;

	if (!cmd->cust_argp)
		return -EFAULT;

	err = copy_to_user(cmd->cust_argp, &segments_per_section, sizeof(unsigned int));
	if (unlikely(err))
		pr_err("%s, copy_to_user failed, err = %d\n", __func__, err);

	return err;
}

static int mas_blk_get_mapping_pos(struct block_device *bdev,
	struct blkdev_cmd *cmd)
{
	int ret;
	struct stor_dev_mapping_partition mapping_info;

	ret = mas_blk_device_read_mapping_partition(bdev, &mapping_info);
	if (ret) {
		pr_err("%s, blk_device_config_mapping_partition failed %d\n",
			__func__, ret);

		return ret;
	}

	if (!cmd->cust_argp)
		return -EFAULT;

	ret = copy_to_user(cmd->cust_argp, &mapping_info,
		sizeof(struct stor_dev_mapping_partition));
	if (unlikely(ret))
		pr_err("%s, copy_from_user failed %d\n", __func__, ret);

	return ret;
}

static int mas_blk_set_mapping_pos(struct block_device *bdev,
	struct blkdev_cmd *cmd)
{
	int ret;
	struct stor_dev_mapping_partition mapping_info;

	if (!cmd->cust_argp)
		return -EFAULT;

	ret = copy_from_user(&mapping_info, cmd->cust_argp,
		sizeof(struct stor_dev_mapping_partition));
	if (unlikely(ret)) {
		pr_err("%s, copy_from_user failed, err = %d\n",
			__func__, ret);

		return ret;
	}

	ret = mas_blk_device_config_mapping_partition(bdev, &mapping_info);
	if (ret)
		pr_err("%s, blk_device_config_mapping_partition failed %d\n",
			__func__, ret);

	return ret;
}

int mas_blk_close_section(struct block_device *bdev,
	struct blkdev_cmd *cmd)
{
	int ret;
	struct stor_dev_reset_ftl reset_ftl_info;

	if (!cmd->cust_argp)
		return -EFAULT;

	ret = copy_from_user(&reset_ftl_info, cmd->cust_argp,
		sizeof(struct stor_dev_reset_ftl));
	if (unlikely(ret)) {
		pr_err("%s, copy_from_user failed, err = %d\n",
			__func__, ret);

		return ret;
	}

	ret = mas_blk_device_close_section(bdev, &reset_ftl_info);
	if (ret)
		pr_err("%s, blk_device_reset_ftl failed, err = %d\n",
			__func__, ret);

	return ret;
}

void mas_blk_insert_section_list(struct block_device *bdev,
	unsigned int start_blkaddr, int stream_type, int flash_mode)
{
	sector_t section_start_lba;
	unsigned long flags = 0;
	struct hd_struct *p = NULL;
	struct request_queue *q = NULL;
	struct blk_dev_lld *lld = NULL;
	struct unistore_section_info *section_info = NULL;

	if (!bdev)
		return;

	if ((!stream_type) || stream_type > MAX_WRITE_STREAM_TYPE)
		return;

	q = bdev_get_queue(bdev);
	if (!blk_queue_query_unistore_enable(q))
		return;

	lld = mas_blk_get_lld(q);
	if (!lld || !lld->mas_sec_size) {
		pr_err("%s - section size error: %u\n", __func__, lld->mas_sec_size);
		return;
	}

	rcu_read_lock();
	p = __disk_get_part(bdev->bd_disk, bdev->bd_partno);
	if (!p) {
		pr_err("%s - disk get part failed\n", __func__);
		rcu_read_unlock();
		return;
	}
	section_start_lba = start_blkaddr + (p->start_sect >> 3);
	rcu_read_unlock();

	if (section_start_lba % (lld->mas_sec_size))
		return;

#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
	if (mas_blk_unistore_debug_en())
		pr_err("%s, start_blkaddr = %u,"
			"section_start_lba = 0x%llx - 0x%llx, stream_type = %d\n",
			__func__, start_blkaddr, section_start_lba / (lld->mas_sec_size),
			section_start_lba % (lld->mas_sec_size), stream_type);
#endif
	section_info = kmalloc(sizeof(struct unistore_section_info), GFP_KERNEL);
	if (unlikely(!section_info)) {
		pr_err("%s, kmalloc fail\n", __func__);
		return;
	}

	section_info->section_start_lba = section_start_lba;
	section_info->slc_mode = (flash_mode == 1) ? true : false;

	spin_lock_irqsave(&lld->expected_lba_lock[stream_type - 1], flags);
	list_add_tail(&section_info->section_list, &lld->section_list[stream_type - 1]);
	spin_unlock_irqrestore(&lld->expected_lba_lock[stream_type - 1], flags);
}

static int mas_blk_set_max_meta_mapping_pos(struct block_device *bdev)
{
	int ret;
	int i;
	struct stor_dev_mapping_partition mapping_info;
	unsigned int sec_num = 0;
	unsigned int sec_size = 0;
	unsigned int total_size = 0;

	struct request_queue *q = bdev_get_queue(bdev);
	if (!blk_queue_query_unistore_enable(q))
		return -EFAULT;

	ret = mas_blk_device_read_section(bdev, &sec_size);
	if (unlikely(ret)) {
		pr_err("%s, read sec size failed %d\n", __func__, ret);
		return ret;
	}

	ret = mas_blk_device_read_mapping_partition(bdev, &mapping_info);
	if (unlikely(ret)) {
		pr_err("%s, read_mapping_partition failed %d\n", __func__, ret);
		return ret;
	}

	if (sec_size == 0x9000) /* 144M */
		sec_num = 0xC8; /* section number = 200 */
	else if (sec_size == 0x12000) /* 288M */
		sec_num = 0x64; /* section number = 100 */
	else
		return -EFAULT;

	for (i = 0; i < PARTITION_TYPE_MAX; i++)
		total_size += mapping_info.partion_size[i];

	/* first partition */
	mapping_info.partion_start[PARTITION_TYPE_META0] = 0;
	mapping_info.partion_size[PARTITION_TYPE_META0] = sec_num;

	mapping_info.partion_start[PARTITION_TYPE_USER0] = sec_num;
	mapping_info.partion_size[PARTITION_TYPE_USER0] = total_size - sec_num;

	/* second partition */
	mapping_info.partion_start[PARTITION_TYPE_META1] = 0;
	mapping_info.partion_size[PARTITION_TYPE_META1] = 0;

	mapping_info.partion_start[PARTITION_TYPE_USER1] = 0;
	mapping_info.partion_size[PARTITION_TYPE_USER1] = 0;

	ret = mas_blk_device_config_mapping_partition(bdev, &mapping_info);
	if (ret)
		pr_err("%s, config mapping partition failed %d\n", __func__, ret);

	return ret;
}

int mas_blk_read_op_size(struct block_device *bdev,
	struct blkdev_cmd *cmd)
{
	int err;
	int op_size = 0;

	err = mas_blk_device_read_op_size(bdev, &op_size);
	if (unlikely(err))
		return err;

	if (!cmd->cust_argp)
		return -EFAULT;

	err = copy_to_user(cmd->cust_argp, &op_size, sizeof(int));
	if (unlikely(err))
		pr_err("%s, copy_to_user failed, err = %d\n", __func__, err);

	return err;
}
#endif /* CONFIG_MAS_UNISTORE_PRESERVE */

static int mas_blk_get_device_unistore_enabled(
	struct block_device *bdev, struct blkdev_cmd *cmd)
{
	int err;
	uint8_t enabled = 0;
	struct request_queue *q = bdev_get_queue(bdev);

	if (blk_queue_query_unistore_enable(q))
		enabled = 1;

	if (!cmd->cust_argp)
		return -EFAULT;

	err = copy_to_user(cmd->cust_argp, &enabled, sizeof(uint8_t));
	if (unlikely(err))
		pr_err("%s, copy_to_user failed, err = %d\n", __func__, err);

	return err;
}

int mas_blk_cust_ioctl(struct block_device *bdev, struct blkdev_cmd __user *arg)
{
	struct blkdev_cmd cmd;

	if (!arg)
		return -EFAULT;

	if (copy_from_user(&cmd, arg, sizeof(struct blkdev_cmd))) {
		pr_err("%s copy_from_user failed\n", __func__);
		return -EFAULT;
	}

	pr_err("%s! cmd = 0x%lx\n", __func__, cmd.cmd);
	switch (cmd.cmd) {
#ifdef CONFIG_MAS_UNISTORE_PRESERVE
	case CUST_BLKDEV_SET_STREAM_ID:
		return mas_blk_set_streamid(bdev->bd_part, &cmd);
	case CUST_BLKDEV_GET_OPEN_PTR:
		return mas_blk_get_open_ptr(bdev, &cmd);
	case CUST_BLKDEV_GET_SEGS_PER_SEC:
		return mas_blk_get_segments_per_section(bdev, &cmd);
	case CUST_BLKDEV_RESET_FTL:
		return mas_blk_device_reset_ftl(bdev);
	case CUST_BLKDEV_SET_MAPPING_POS:
		return mas_blk_set_mapping_pos(bdev, &cmd);
	case CUST_BLKDEV_GET_OP_SIZE:
		return mas_blk_read_op_size(bdev, &cmd);
	case CUST_BLKDEV_GET_MAPPING_POS:
		return mas_blk_get_mapping_pos(bdev, &cmd);
	case CUST_BLKDEV_CLOSE_SECTION:
		return mas_blk_close_section(bdev, &cmd);
	case CUST_BLKDEV_FS_SYNC_DONE:
		return mas_blk_fs_sync_done(bdev);
	case CUST_BLKDEV_SET_MAX_META_MAPPING_POS:
		return mas_blk_set_max_meta_mapping_pos(bdev);
#endif
	case CUST_BLKDEV_GET_UNISTORE_ENABLE:
		return mas_blk_get_device_unistore_enabled(bdev, &cmd);
	default:
		break;
	}
	return 0;
}
