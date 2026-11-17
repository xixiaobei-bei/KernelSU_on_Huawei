/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description: unistore implement
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

#include <linux/blkdev.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_device.h>

static int scsi_dev_pwron_info_sync(struct request_queue *q,
	struct stor_dev_pwron_info *stor_info, unsigned int rescue_seg_size)
{
	struct scsi_device *sdev = q->queuedata;

	if (!sdev || sdev->type != TYPE_DISK ||
		!sdev->host->hostt->dev_pwron_info_sync)
		return -EINVAL;

	if (sdev->host->host_self_blocked || sdev->sdev_state != SDEV_RUNNING)
		return -EINVAL;

	return sdev->host->hostt->dev_pwron_info_sync(sdev, stor_info,
			rescue_seg_size);
}

static int scsi_stream_oob_info_fetch(struct request_queue *q,
	struct stor_dev_stream_info stream_info,
	unsigned int oob_entry_cnt,
	struct stor_dev_stream_oob_info *oob_info)
{
	struct scsi_device *sdev = q->queuedata;

	if (!sdev || sdev->type != TYPE_DISK ||
		!sdev->host->hostt->dev_stream_oob_info_fetch)
		return -EINVAL;

	if (sdev->host->host_self_blocked || sdev->sdev_state != SDEV_RUNNING)
		return -EINVAL;

	return sdev->host->hostt->dev_stream_oob_info_fetch(sdev, stream_info,
		oob_entry_cnt, oob_info);
}

static int scsi_dev_reset_ftl(struct request_queue *q,
	struct stor_dev_reset_ftl *reset_ftl_info)
{
	struct scsi_device *sdev = q->queuedata;

	if (!sdev || sdev->type != TYPE_DISK || !reset_ftl_info ||
		!sdev->host->hostt->dev_reset_ftl)
		return -EINVAL;

	if (sdev->host->host_self_blocked || sdev->sdev_state != SDEV_RUNNING)
		return -EINVAL;

	return sdev->host->hostt->dev_reset_ftl(sdev, reset_ftl_info);
}

static int scsi_dev_read_section(struct request_queue *q,
	unsigned int *section_size)
{
	struct scsi_device *sdev = q->queuedata;

	if (!sdev || sdev->type != TYPE_DISK || !section_size ||
		!sdev->host->hostt->dev_read_section)
		return -EINVAL;

	if (sdev->host->host_self_blocked || sdev->sdev_state != SDEV_RUNNING)
		return -EINVAL;

	return sdev->host->hostt->dev_read_section(sdev, section_size);
}

static int scsi_dev_read_lrb_in_use(struct request_queue *q,
	unsigned long *lrb_in_use)
{
	struct scsi_device *sdev = q->queuedata;

	if (!sdev || sdev->type != TYPE_DISK || !lrb_in_use ||
		!sdev->host->hostt->dev_read_lrb_in_use)
		return -EINVAL;

	if (sdev->host->host_self_blocked || sdev->sdev_state != SDEV_RUNNING)
		return -EINVAL;

	return sdev->host->hostt->dev_read_lrb_in_use(sdev, lrb_in_use);
}

static int scsi_dev_read_op_size(struct request_queue *q,
	int *op_size)
{
	struct scsi_device *sdev = q->queuedata;

	if (!sdev || sdev->type != TYPE_DISK || !op_size ||
		!sdev->host->hostt->dev_read_op_size)
		return -EINVAL;

	if (sdev->host->host_self_blocked || sdev->sdev_state != SDEV_RUNNING)
		return -EINVAL;

	return sdev->host->hostt->dev_read_op_size(sdev, op_size);
}

static int scsi_dev_config_mapping_partition(
	struct request_queue *q,
	struct stor_dev_mapping_partition *mapping_info)
{
	struct scsi_device *sdev = q->queuedata;

	if (!sdev || sdev->type != TYPE_DISK || !mapping_info ||
		!sdev->host->hostt->dev_config_mapping_partition)
		return -EINVAL;
	if (sdev->host->host_self_blocked || sdev->sdev_state != SDEV_RUNNING)
		return -EINVAL;

	return sdev->host->hostt->dev_config_mapping_partition(sdev,
		mapping_info);
}

static int scsi_dev_read_mapping_partition(
	struct request_queue *q,
	struct stor_dev_mapping_partition *mapping_info)
{
	struct scsi_device *sdev = q->queuedata;

	if (!sdev || sdev->type != TYPE_DISK || !mapping_info ||
		!sdev->host->hostt->dev_read_mapping_partition)
		return -EINVAL;

	if (sdev->host->host_self_blocked || sdev->sdev_state != SDEV_RUNNING)
		return -EINVAL;

	return sdev->host->hostt->dev_read_mapping_partition(sdev,
		mapping_info);
}

static int scsi_dev_fs_sync_done(struct request_queue *q)
{
	struct scsi_device *sdev = q->queuedata;

	if (!sdev || sdev->type != TYPE_DISK ||
		!sdev->host->hostt->dev_fs_sync_done)
		return -EINVAL;

	if (sdev->host->host_self_blocked || sdev->sdev_state != SDEV_RUNNING)
		return -EINVAL;

	return sdev->host->hostt->dev_fs_sync_done(sdev);
}

static int scsi_dev_data_move(struct request_queue *q,
				struct stor_dev_data_move_info *data_move_info)
{
	struct scsi_device *sdev = q->queuedata;

	if (!sdev || sdev->type != TYPE_DISK ||
		!sdev->host->hostt->dev_data_move)
		return -EINVAL;

	if (sdev->host->host_self_blocked || sdev->sdev_state != SDEV_RUNNING)
		return -EINVAL;

	return sdev->host->hostt->dev_data_move(sdev, data_move_info);
}

static int scsi_dev_slc_mode_configuration(struct request_queue *q,
	int *status)
{
	struct scsi_device *sdev = q->queuedata;

	if (!sdev || sdev->type != TYPE_DISK ||
		!sdev->host->hostt->dev_slc_mode_configuration)
		return -EINVAL;

	if (sdev->host->host_self_blocked || sdev->sdev_state != SDEV_RUNNING)
		return -EINVAL;

	return sdev->host->hostt->dev_slc_mode_configuration(sdev, status);
}

static int scsi_dev_sync_read_verify(struct request_queue *q,
	struct stor_dev_sync_read_verify_info *verify_info)
{
	struct scsi_device *sdev = q->queuedata;

	if (!sdev || sdev->type != TYPE_DISK ||
		!sdev->host->hostt->dev_sync_read_verify)
		return -EINVAL;

	if (sdev->host->host_self_blocked || sdev->sdev_state != SDEV_RUNNING)
		return -EINVAL;

	return sdev->host->hostt->dev_sync_read_verify(sdev, verify_info);
}

static int scsi_dev_get_program_size(struct request_queue *q,
	struct stor_dev_program_size *program_size)
{
	struct scsi_device *sdev = q->queuedata;
	if (sdev->type != TYPE_DISK ||
		!sdev->host->hostt->dev_get_program_size)
		return -EINVAL;
	if (sdev->host->host_self_blocked || sdev->sdev_state != SDEV_RUNNING)
		return -EINVAL;
	return sdev->host->hostt->dev_get_program_size(sdev, program_size);
}

static int scsi_dev_get_bad_block_info(struct request_queue *q,
	struct stor_dev_bad_block_info *bad_block_info)
{
	struct scsi_device *sdev = q->queuedata;

	if (sdev->type != TYPE_DISK ||
		!sdev->host->hostt->dev_get_bad_block_info)
		return -EINVAL;

	if (sdev->host->host_self_blocked || sdev->sdev_state != SDEV_RUNNING)
		return -EINVAL;

	return sdev->host->hostt->dev_get_bad_block_info(sdev, bad_block_info);
}

#ifdef CONFIG_MAS_DEBUG_FS
static int scsi_dev_rescue_block_inject_data(
	struct request_queue *q, unsigned int lba)
{
	struct scsi_device *sdev = q->queuedata;

	if (!sdev || sdev->type != TYPE_DISK ||
		!sdev->host->hostt->dev_rescue_block_inject_data)
		return -EINVAL;

	if (sdev->host->host_self_blocked || sdev->sdev_state != SDEV_RUNNING)
		return -EINVAL;

	return sdev->host->hostt->dev_rescue_block_inject_data(sdev, lba);
}

static int scsi_dev_bad_block_error_inject(struct request_queue *q,
		unsigned char bad_slc_cnt, unsigned char bad_tlc_cnt)
{
	struct scsi_device *sdev = q->queuedata;

	if (!sdev || sdev->type != TYPE_DISK ||
		!sdev->host->hostt->dev_bad_block_error_inject)
		return -EINVAL;

	if (sdev->host->host_self_blocked || sdev->sdev_state != SDEV_RUNNING)
		return -EINVAL;

	return sdev->host->hostt->dev_bad_block_error_inject(sdev,
		bad_slc_cnt, bad_tlc_cnt);
}
#endif

static int scsi_dev_bad_block_notify_register(
	struct request_queue *q, void (*func)(struct Scsi_Host *host,
		struct stor_dev_bad_block_info *bad_block_info))
{
	struct scsi_device *sdev = q->queuedata;

	if (!sdev || !sdev->host->hostt->dev_bad_block_notify_register)
		return -EINVAL;

	return sdev->host->hostt->dev_bad_block_notify_register(sdev, func);
}

void scsi_dev_unistore_register(struct Scsi_Host *shost)
{
	mas_blk_mq_tagset_pwron_info_sync_register(
		&shost->tag_set, shost->hostt->dev_pwron_info_sync ?
		scsi_dev_pwron_info_sync : NULL);
	mas_blk_mq_tagset_stream_oob_info_register(
		&shost->tag_set, shost->hostt->dev_stream_oob_info_fetch ?
		scsi_stream_oob_info_fetch : NULL);
	mas_blk_mq_tagset_reset_ftl_register(
		&shost->tag_set, shost->hostt->dev_reset_ftl ?
		scsi_dev_reset_ftl : NULL);
	mas_blk_mq_tagset_read_section_register(
		&shost->tag_set, shost->hostt->dev_read_section ?
		scsi_dev_read_section : NULL);
	mas_blk_mq_tagset_read_lrb_in_use_register(
		&shost->tag_set, shost->hostt->dev_read_lrb_in_use ?
		scsi_dev_read_lrb_in_use : NULL);
	mas_blk_mq_tagset_read_op_size_register(
		&shost->tag_set, shost->hostt->dev_read_op_size ?
		scsi_dev_read_op_size : NULL);
	mas_blk_mq_tagset_config_mapping_partition_register(
		&shost->tag_set, shost->hostt->dev_config_mapping_partition ?
		scsi_dev_config_mapping_partition : NULL);
	mas_blk_mq_tagset_read_mapping_partition_register(
		&shost->tag_set,
		shost->hostt->dev_read_mapping_partition ?
		scsi_dev_read_mapping_partition : NULL);
	mas_blk_mq_tagset_fs_sync_done_register(
		&shost->tag_set, shost->hostt->dev_fs_sync_done ?
		scsi_dev_fs_sync_done : NULL);
	mas_blk_mq_tagset_data_move_register(
		&shost->tag_set, shost->hostt->dev_data_move ?
		scsi_dev_data_move : NULL);
	mas_blk_mq_tagset_slc_mode_configuration_register(
		&shost->tag_set, shost->hostt->dev_slc_mode_configuration ?
		scsi_dev_slc_mode_configuration : NULL);
	mas_blk_mq_tagset_sync_read_verify_register(
		&shost->tag_set, shost->hostt->dev_sync_read_verify ?
		scsi_dev_sync_read_verify : NULL);
	mas_blk_mq_tagset_get_bad_block_info_register(
		&shost->tag_set, shost->hostt->dev_get_bad_block_info ?
		scsi_dev_get_bad_block_info : NULL);
	mas_blk_mq_tagset_bad_block_notify_register(
		&shost->tag_set, shost->hostt->dev_bad_block_notify_register ?
		scsi_dev_bad_block_notify_register : NULL);
	mas_blk_mq_tagset_get_program_size_register(
		&shost->tag_set, shost->hostt->dev_get_program_size ?
		scsi_dev_get_program_size : NULL);
#ifdef CONFIG_MAS_DEBUG_FS
	mas_blk_mq_tagset_rescue_block_inject_data_register(
		&shost->tag_set, shost->hostt->dev_rescue_block_inject_data ?
		scsi_dev_rescue_block_inject_data : NULL);
	mas_blk_mq_tagset_bad_block_error_inject_register(
		&shost->tag_set, shost->hostt->dev_bad_block_error_inject ?
		scsi_dev_bad_block_error_inject : NULL);
#endif
}
