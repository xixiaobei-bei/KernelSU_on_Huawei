/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 * Description: mas block unistore lld function registration
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
#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>
#include "blk.h"
#include "dsm_block.h"

void mas_blk_mq_tagset_pwron_info_sync_register(
	struct blk_mq_tag_set *tag_set, lld_dev_pwron_info_sync_fn func)
{
	if (tag_set)
		tag_set->lld_func.unistore_ops.dev_pwron_info_sync = func;
}

void mas_blk_mq_tagset_stream_oob_info_register(
	struct blk_mq_tag_set *tag_set, lld_dev_stream_oob_info_fetch_fn func)
{
	if (tag_set)
		tag_set->lld_func.unistore_ops.dev_stream_oob_info_fetch = func;
}

void mas_blk_mq_tagset_reset_ftl_register(
	struct blk_mq_tag_set *tag_set, lld_dev_reset_ftl_fn func)
{
	if (tag_set)
		tag_set->lld_func.unistore_ops.dev_reset_ftl = func;
}

void mas_blk_mq_tagset_read_section_register(
	struct blk_mq_tag_set *tag_set, lld_dev_read_section_fn func)
{
	if (tag_set)
		tag_set->lld_func.unistore_ops.dev_read_section = func;
}

void mas_blk_mq_tagset_read_lrb_in_use_register(
	struct blk_mq_tag_set *tag_set, lld_dev_read_lrb_in_use_fn func)
{
	if (tag_set)
		tag_set->lld_func.unistore_ops.dev_read_lrb_in_use = func;
}

void mas_blk_mq_tagset_config_mapping_partition_register(
	struct blk_mq_tag_set *tag_set,
	lld_dev_config_mapping_partition_fn func)
{
	if (tag_set)
		tag_set->lld_func.unistore_ops.dev_config_mapping_partition = func;
}

void mas_blk_mq_tagset_read_mapping_partition_register(
	struct blk_mq_tag_set *tag_set, lld_dev_read_mapping_partition_fn func)
{
	if (tag_set)
		tag_set->lld_func.unistore_ops.dev_read_mapping_partition = func;
}

void mas_blk_mq_tagset_fs_sync_done_register(
	struct blk_mq_tag_set *tag_set, lld_dev_fs_sync_done_fn func)
{
	if (tag_set)
		tag_set->lld_func.unistore_ops.dev_fs_sync_done = func;
}

#ifdef CONFIG_MAS_DEBUG_FS
void mas_blk_mq_tagset_rescue_block_inject_data_register(
	struct blk_mq_tag_set *tag_set, lld_dev_rescue_block_inject_data_fn func)
{
	if (tag_set)
		tag_set->lld_func.unistore_ops.dev_rescue_block_inject_data = func;
}

void mas_blk_mq_tagset_bad_block_error_inject_register(
	struct blk_mq_tag_set *tag_set, lld_dev_bad_block_error_inject_fn func)
{
	if (tag_set)
		tag_set->lld_func.unistore_ops.dev_bad_block_err_inject = func;
}
#endif

void mas_blk_mq_tagset_data_move_register(
	struct blk_mq_tag_set *tag_set, lld_dev_data_move_fn func)
{
	if (tag_set)
		tag_set->lld_func.unistore_ops.dev_data_move = func;
}

void mas_blk_mq_tagset_slc_mode_configuration_register(
	struct blk_mq_tag_set *tag_set, lld_dev_slc_mode_configuration_fn func)
{
	if (tag_set)
		tag_set->lld_func.unistore_ops.dev_slc_mode_configuration = func;
}

void mas_blk_mq_tagset_sync_read_verify_register(
	struct blk_mq_tag_set *tag_set, lld_dev_sync_read_verify_fn func)
{
	if (tag_set)
		tag_set->lld_func.unistore_ops.dev_sync_read_verify = func;
}

void mas_blk_mq_tagset_get_bad_block_info_register(
	struct blk_mq_tag_set *tag_set, lld_dev_get_bad_block_info_fn func)
{
	if (tag_set)
		tag_set->lld_func.unistore_ops.dev_get_bad_block_info = func;
}

void mas_blk_bad_block_notify_register(
	struct block_device *bi_bdev, blk_dev_bad_block_notify_fn func, void *param_data)
{
	struct request_queue *q = NULL;
	struct blk_dev_lld *lld_fn = NULL;

	if (!bi_bdev)
		return;

	q = bdev_get_queue(bi_bdev);
	lld_fn = mas_blk_get_lld(q);
	if (!lld_fn)
		return;

	lld_fn->unistore_ops.dev_bad_block_notfiy_fn = func;
	lld_fn->unistore_ops.dev_bad_block_notfiy_param_data = param_data;
}

void mas_blk_mq_tagset_bad_block_notify_register(
	struct blk_mq_tag_set *tag_set,
	lld_dev_bad_block_notify_register_fn func)
{
	if (tag_set)
		tag_set->lld_func.unistore_ops.dev_bad_block_notify_register = func;
}

void mas_blk_mq_tagset_get_program_size_register(
	struct blk_mq_tag_set *tag_set, lld_dev_get_program_size_fn func)
{
	if (tag_set)
		tag_set->lld_func.unistore_ops.dev_get_program_size = func;
}

void mas_blk_mq_tagset_set_bkops(
	struct request_queue *q, struct mas_bkops *ufs_bkops)
{
	struct blk_dev_lld *lld = mas_blk_get_lld(q);
	if (!lld)
		return;

	lld->bkops = ufs_bkops;
}

void mas_blk_mq_tagset_read_op_size_register(
	struct blk_mq_tag_set *tag_set, lld_dev_read_op_size_fn func)
{
	if (tag_set)
		tag_set->lld_func.unistore_ops.dev_read_op_size = func;
}
