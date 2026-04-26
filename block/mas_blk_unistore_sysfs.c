/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 * Description: mas block unistore debugfs
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
#include "blk.h"
#include "dsm_block.h"

#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
static unsigned int oob_lba = 0;
static int tst_data_move_num = 1;
static int data_move_test_src_lun = 3;
static int stream_oob_test_length = 6;
static unsigned int tst_cp_verify_l4k = 0;
static unsigned int tst_cp_open_l4k = 0;
static unsigned short tst_tlc_total_block_num = 0;
static unsigned char tst_tlc_bad_block_num = 0;
static unsigned char real_bad_tlc_cnt = 0;
static int recovery_debug_on = 0;

ssize_t mas_queue_device_pwron_info_show(
	struct request_queue *q, char *page)
{
	ssize_t offset = 0;
	int ret = -EPERM;
	unsigned int i;
	struct stor_dev_pwron_info stor_info;
	struct blk_dev_lld *lld_fn = mas_blk_get_lld(q);

	if (!lld_fn || !lld_fn->unistore_ops.dev_pwron_info_sync)
		return ret;

	stor_info.rescue_seg = vmalloc(sizeof(unsigned int) * MAX_RESCUE_SEG_CNT);
	if (unlikely(!stor_info.rescue_seg)) {
		pr_err("%s: alloc mem failed\n", __func__);
		return -ENOMEM;
	}

	ret = lld_fn->unistore_ops.dev_pwron_info_sync(q, &stor_info,
		sizeof(unsigned int) * MAX_RESCUE_SEG_CNT);

	if (unlikely(ret)) {
		pr_err("%s dev_pwron_info_sync return err! <%d>\n", __func__, ret);
		goto free_rescue_seg;
	}

	for (i = 0; i < STREAM_NUM; i++)
		offset += snprintf(page + offset, PAGE_SIZE - offset,
			"stream_addr[%u] = %u:\n",
			i, stor_info.dev_stream_addr[i]);

	for (i = 0; i < stor_info.rescue_seg_cnt; i++)
		offset += snprintf(page + offset, PAGE_SIZE - offset,
			"rescue_seg[%u] = %u:\n",
			i, stor_info.rescue_seg[i]);

	offset += snprintf(page + offset, PAGE_SIZE - offset,
			"pe_limit_status = %u:\n", stor_info.pe_limit_status);

	for (i = 0; i < DATA_MOVE_STREAM_NUM; i++)
		offset += snprintf(page + offset, PAGE_SIZE - offset,
			"dm_stream_addr[%u] = %u:\n",
			i, stor_info.dm_stream_addr[i]);

	for (i = 0; i < STREAM_NUM; i++)
		offset += snprintf(page + offset, PAGE_SIZE - offset,
			"stream_lun_info[%u] = %u:\n",
			i, stor_info.stream_lun_info[i]);

	for (i = 0; i < DATA_MOVE_STREAM_NUM; i++)
		offset += snprintf(page + offset, PAGE_SIZE - offset,
			"dm_lun_info[%u] = %u:\n",
			i, stor_info.dm_lun_info[i]);

	offset += snprintf(page + offset, PAGE_SIZE - offset,
			"io_slc_mode_status = %u:\n", stor_info.io_slc_mode_status);

	offset += snprintf(page + offset, PAGE_SIZE - offset,
			"dm_slc_mode_status = %u:\n", stor_info.dm_slc_mode_status);
free_rescue_seg:
	vfree(stor_info.rescue_seg);

	return offset;
}

ssize_t mas_queue_stream_oob_info_show(
	struct request_queue *q, char *page)
{
	const unsigned int output_cnt = stream_oob_test_length;
	ssize_t offset = 0;
	int ret = -EPERM;
	unsigned int j;
	unsigned int lba;
	struct stor_dev_pwron_info stor_info;
	struct stor_dev_stream_info stream_info;
	struct stor_dev_stream_oob_info *oob_info = NULL;
	struct blk_dev_lld *lld_fn = NULL;

	lld_fn = mas_blk_get_lld(q);

	if (!lld_fn || !lld_fn->unistore_ops.dev_pwron_info_sync ||
		!lld_fn->unistore_ops.dev_stream_oob_info_fetch)
		goto exit;

	stor_info.rescue_seg = vmalloc(sizeof(unsigned int) * MAX_RESCUE_SEG_CNT);
	if (unlikely(!stor_info.rescue_seg)) {
		pr_err("%s: alloc mem failed\n", __func__);
		ret = -ENOMEM;
		goto exit;
	}

	ret = lld_fn->unistore_ops.dev_pwron_info_sync(q, &stor_info,
			sizeof(unsigned int) * MAX_RESCUE_SEG_CNT);

	if (unlikely(ret)) {
		pr_err("%s dev_pwron_info_sync return err! <%d>\n", __func__, ret);
		goto free_rescue_seg;
	}

	oob_info = kzalloc(sizeof(struct stor_dev_stream_oob_info) * output_cnt, GFP_KERNEL);
	if (unlikely(!oob_info)) {
		pr_err("%s alloc mem fail\n", __func__);
		ret = -ENOMEM;
		goto free_rescue_seg;
	}

	lba = oob_lba;
	stream_info.stream_id = DATA_MOVE_STREAM_NUM;
	stream_info.stream_start_lba = lba;
	stream_info.dm_stream = false;

	ret = lld_fn->unistore_ops.dev_stream_oob_info_fetch(q,
		stream_info, output_cnt, oob_info);
	if (unlikely(ret)) {
		pr_err("%s dev_stream_oob_info_fetch return err! <%d>\n",
							__func__, ret);
		goto free_oob_info;
	}
	for (j = 0; j < output_cnt; j++)
		offset += snprintf(page + offset, PAGE_SIZE - offset,
				"stream[2]: ino %lu offset %lu\n",
				oob_info[j].data_ino, oob_info[j].data_idx);

free_oob_info:
	kfree(oob_info);
free_rescue_seg:
	vfree(stor_info.rescue_seg);
exit:
	return ret ? ret : offset;
}

ssize_t mas_queue_device_reset_ftl_store(
	struct request_queue *q, const char *page, size_t count)
{
	ssize_t ret;
	int re = 0;
	unsigned long val;
	struct blk_dev_lld *lld_fn = mas_blk_get_lld(q);
	struct stor_dev_reset_ftl reset_ftl_info = {0};

	ret = queue_var_store(&val, page, count);
	if (ret < 0)
		return count;

	if (!val || !lld_fn || !lld_fn->unistore_ops.dev_reset_ftl)
		return count;

	re = lld_fn->unistore_ops.dev_reset_ftl(q, &reset_ftl_info);

	if (unlikely(re))
		pr_err("%s dev_reset_ftl return err! <%d>\n", __func__, re);

	return count;
}

ssize_t mas_queue_device_read_section_show(
	struct request_queue *q, char *page)
{
	int ret = -EPERM;
	unsigned int section_size = 0;
	struct blk_dev_lld *lld_fn = mas_blk_get_lld(q);

	if (!lld_fn || !lld_fn->unistore_ops.dev_read_section)
		return ret;

	ret = lld_fn->unistore_ops.dev_read_section(q, &section_size);

	if (unlikely(ret)) {
		pr_err("%s read section return err! <%d>\n", __func__, ret);
		return ret;
	}

	return snprintf(page, PAGE_SIZE, "section size: %u\n", section_size);
}

ssize_t mas_queue_device_config_mapping_partition_show(
	struct request_queue *q, char *page)
{
	int ret = -EPERM;
	ssize_t offset = 0;
	unsigned int i = 0;
	struct blk_dev_lld *lld_fn = mas_blk_get_lld(q);
	struct stor_dev_mapping_partition mapping_info;

	if (!lld_fn || !lld_fn->unistore_ops.dev_read_mapping_partition)
		return ret;

	ret = lld_fn->unistore_ops.dev_read_mapping_partition(q, &mapping_info);

	if (unlikely(ret)) {
		pr_err("%s print mapping partition err! <%d>\n", __func__, ret);
		return ret;
	}

	for (i = 0; i < PARTITION_TYPE_MAX; i++)
		offset += snprintf(page + offset, PAGE_SIZE - offset,
			"mapping_info.partion_start[%u] = %u:\n",
			i, mapping_info.partion_start[i]);

	for (i = 0; i < PARTITION_TYPE_MAX; i++)
		offset += snprintf(page + offset, PAGE_SIZE - offset,
			"mapping_info.partion_size[%u] = %u:\n",
			i, mapping_info.partion_size[i]);

	return offset;
}

ssize_t mas_queue_device_config_mapping_partition_store(
	struct request_queue *q, const char *page, size_t count)
{
	int i;
	int re;
	ssize_t ret;
	unsigned long val;
	unsigned int total_size = 0;
	struct blk_dev_lld *lld_fn = mas_blk_get_lld(q);
	struct stor_dev_mapping_partition mapping_info;

	ret = queue_var_store(&val, page, count);
	if (ret < 0)
		return count;

	if (!lld_fn || !lld_fn->unistore_ops.dev_read_mapping_partition ||
		!lld_fn->unistore_ops.dev_config_mapping_partition)
		return count;

	ret = lld_fn->unistore_ops.dev_read_mapping_partition(q, &mapping_info);

	if (unlikely(ret)) {
		pr_err("%s, read_mapping_partition failed %ld\n", __func__, ret);
		return count;
	}

	pr_err("%s 2M map area begin from %llu\n", __func__, val);

	for (i = 0; i < PARTITION_TYPE_MAX; i++)
		total_size += mapping_info.partion_size[i];

	if (val > total_size)
		return count;

	/* first partition */
	mapping_info.partion_start[PARTITION_TYPE_META0] = 0;
	mapping_info.partion_size[PARTITION_TYPE_META0] = val;

	mapping_info.partion_start[PARTITION_TYPE_USER0] = val;
	mapping_info.partion_size[PARTITION_TYPE_USER0] = total_size - val;

	/* second partition */
	mapping_info.partion_start[PARTITION_TYPE_META1] = 0;
	mapping_info.partion_size[PARTITION_TYPE_META1] = 0;

	mapping_info.partion_start[PARTITION_TYPE_USER1] = 0;
	mapping_info.partion_size[PARTITION_TYPE_USER1] = 0;

	re = lld_fn->unistore_ops.dev_config_mapping_partition(q, &mapping_info);

	if (unlikely(re))
		pr_err("%s configure mapping partition err ! <%d>\n", __func__, re);

	return count;
}

ssize_t mas_queue_device_fs_sync_done_store(
	struct request_queue *q, const char *page, size_t count)
{
	ssize_t ret;
	int re = 0;
	unsigned long val;
	struct blk_dev_lld *lld_fn = mas_blk_get_lld(q);

	ret = queue_var_store(&val, page, count);
	if (ret < 0)
		return count;

	if (!val || !lld_fn || !lld_fn->unistore_ops.dev_fs_sync_done)
		return count;

	re = lld_fn->unistore_ops.dev_fs_sync_done(q);
	pr_err("%s: exe dev_fs_sync_done re = %d\n", __func__, re);

	return count;
}

ssize_t mas_queue_device_rescue_block_inject_data_store(
	struct request_queue *q, const char *page, size_t count)
{
	ssize_t ret;
	int re = 0;
	unsigned long val;
	struct blk_dev_lld *lld_fn = mas_blk_get_lld(q);

	ret = queue_var_store(&val, page, count);
	if (ret < 0)
		return count;

	if (!val || !lld_fn || !lld_fn->unistore_ops.dev_rescue_block_inject_data)
		return count;

	re = lld_fn->unistore_ops.dev_rescue_block_inject_data(q, (unsigned int)val);
	pr_err("%s rescue_block_inject_data! <lba:%lu> <ret:%d>\n", __func__, val, re);

	return count;
}

static void data_move_store_init(
	struct stor_dev_data_move_info *data_move_info,
	struct stor_dev_data_move_source_addr *source_addr,
	struct stor_dev_data_move_source_inode *source_inode,
	int data_move_num, unsigned long val, unsigned int mas_sec_size)
{
	unsigned int dest_addr = (val / mas_sec_size + 1) * mas_sec_size;
	data_move_info->data_move_total_length = DATA_MOVE_MAX_NUM;
	data_move_info->dest_4k_lba = dest_addr;
	data_move_info->dest_lun_info = 0;
	data_move_info->dest_stream_id = 0;
	data_move_info->dest_blk_mode = 0;
	data_move_info->force_flush_option = 0;
	data_move_info->source_addr_num = data_move_num;
	data_move_info->source_addr = source_addr;
	data_move_info->source_inode_num = DATA_MOVE_MAX_NUM;
	data_move_info->source_inode = source_inode;
	data_move_info->verify_info.next_to_be_verify_4k_lba = 0;
	data_move_info->verify_info.next_available_write_4k_lba = 0;
	data_move_info->verify_info.lun_info = 0;
	data_move_info->verify_info.verify_done_status = 0;
	data_move_info->verify_info.verify_fail_reason = 0;
}

static void data_move_source_addr_init(
	struct stor_dev_data_move_source_addr *source_addr,
	int data_move_num, unsigned long val)
{
	int i;
	int length_sum = 0;
	for (i = 0; i < data_move_num; i++) {
		(source_addr + i)->data_move_source_addr = val +
			i * (DATA_MOVE_MAX_NUM / data_move_num);
		(source_addr + i)->src_lun = data_move_test_src_lun;
		if (i != data_move_num - 1) {
			(source_addr + i)->source_length =
				DATA_MOVE_MAX_NUM / data_move_num;
			length_sum += DATA_MOVE_MAX_NUM / data_move_num;
		} else {
			(source_addr + i)->source_length =
				DATA_MOVE_MAX_NUM - length_sum;
		}
	}
}

static void data_move_store_handler(struct request_queue *q,
	struct stor_dev_data_move_info *data_move_info, unsigned long val)
{
	int ret = 0;
	struct blk_dev_lld *lld_fn = mas_blk_get_lld(q);

	if (val && lld_fn && lld_fn->unistore_ops.dev_data_move) {
		ret = lld_fn->unistore_ops.dev_data_move(q, data_move_info);

		pr_err("%s <data_move_source_addr:%lu> <ret:%d>\n", __func__, val, ret);

		pr_err("%s data move rsp: next_verify=0x%x, next_available=0x%x,"
				"lun_info=%u, status=%u, reason=%u\n", __func__,
			data_move_info->verify_info.next_to_be_verify_4k_lba,
			data_move_info->verify_info.next_available_write_4k_lba,
			data_move_info->verify_info.lun_info,
			data_move_info->verify_info.verify_done_status,
			data_move_info->verify_info.verify_fail_reason);
	}
}

ssize_t mas_queue_device_data_move_store(
	struct request_queue *q, const char *page, size_t count)
{
	ssize_t ret;
	unsigned long val;
	int data_move_num = 1;
	struct stor_dev_data_move_source_addr *source_addr = NULL;
	struct stor_dev_data_move_source_inode *source_inode = NULL;
	struct stor_dev_data_move_info data_move_info;
	unsigned int mas_sec_size = mas_blk_get_sec_size(q);

	if (tst_data_move_num > 0)
		data_move_num = tst_data_move_num;

	ret = queue_var_store(&val, page, count);
	if (ret < 0)
		return count;

	if (!mas_sec_size) {
		pr_err("%s - section size error: %u\n", __func__, mas_sec_size);
		return count;
	}

	source_addr = kmalloc(sizeof(struct stor_dev_data_move_source_addr) *
						data_move_num, GFP_KERNEL);
	if (!source_addr) {
		pr_err("%s, source_addr null\n", __func__);
		return count;
	}

	source_inode = kmalloc(sizeof(struct stor_dev_data_move_source_inode) *
						DATA_MOVE_MAX_NUM, GFP_KERNEL);
	if (!source_inode) {
		pr_err("%s, source_inode null\n", __func__);
		kfree(source_addr);
		return count;
	}

	memset(source_inode, 0, sizeof(*source_inode));
	if (((val / mas_sec_size + 1) * mas_sec_size) - val < DATA_MOVE_MAX_NUM) {
		pr_err("%s, addr space not enough\n", __func__);
		goto exit;
	}

	data_move_source_addr_init(source_addr, data_move_num, val);
	data_move_store_init(&data_move_info, source_addr,
		source_inode, data_move_num, val, mas_sec_size);
	data_move_store_handler(q, &data_move_info, val);

exit:
	kfree(source_addr);
	kfree(source_inode);

	return count;
}

ssize_t mas_queue_device_data_move_num_store(
	struct request_queue *q, const char *page, size_t count)
{
	ssize_t ret;
	unsigned long val;

	ret = queue_var_store(&val, page, count);
	if (ret < 0)
		return count;

	if (val > DATA_MOVE_MAX_NUM)
		return count;

	tst_data_move_num = val;
	return count;
}

ssize_t mas_queue_device_slc_mode_configuration_show(
	struct request_queue *q, char *page)
{
	int ret = -EPERM;
	struct blk_dev_lld *lld_fn = NULL;
	int status = 0;

	lld_fn = mas_blk_get_lld(q);
	if (lld_fn && lld_fn->unistore_ops.dev_slc_mode_configuration) {
		ret = lld_fn->unistore_ops.dev_slc_mode_configuration(q, &status);
		pr_err("%s slc_mode_configuration! <ret:%d> <status:%d>\n",
						__func__, ret, status);
	}

	return snprintf(page, PAGE_SIZE,
		"SLC Mode Configuration ret:%d, status:%d \n", ret, status);
}

ssize_t mas_queue_device_sync_read_verify_show(
	struct request_queue *q, char *page)
{
	ssize_t offset = 0;
	int ret = -EPERM;
	struct blk_dev_lld *lld_fn = mas_blk_get_lld(q);
	struct stor_dev_sync_read_verify_info verify_info;

	verify_info.verify_info.next_to_be_verify_4k_lba = 0;
	verify_info.verify_info.next_available_write_4k_lba = 0;
	verify_info.verify_info.lun_info = 0;
	verify_info.verify_info.verify_done_status = 0;
	verify_info.verify_info.verify_fail_reason = 0;

	verify_info.stream_id = 0;
	verify_info.cp_verify_l4k = tst_cp_verify_l4k;
	verify_info.cp_open_l4k = tst_cp_open_l4k;
	verify_info.cp_cache_l4k = tst_cp_open_l4k;
	verify_info.error_injection = 0;

	if (lld_fn && lld_fn->unistore_ops.dev_sync_read_verify)
		ret = lld_fn->unistore_ops.dev_sync_read_verify(q, &verify_info);

	offset += snprintf(page + offset, PAGE_SIZE - offset,
				"ret = %d\r\n", ret);

	offset += snprintf(page + offset, PAGE_SIZE - offset,
				"next_to_be_verify_4k_lba = 0x%x\r\n",
				verify_info.verify_info.next_to_be_verify_4k_lba);

	offset += snprintf(page + offset, PAGE_SIZE - offset,
				"next_available_write_4k_lba = 0x%x\r\n",
				verify_info.verify_info.next_available_write_4k_lba);

	offset += snprintf(page + offset, PAGE_SIZE - offset,
				"lun_info = %u\r\n", verify_info.verify_info.lun_info);

	offset += snprintf(page + offset, PAGE_SIZE - offset,
				"verify_done_status = %u\r\n",
				verify_info.verify_info.verify_done_status);

	offset += snprintf(page + offset, PAGE_SIZE - offset,
				"verify_fail_reason = %u\r\n",
				verify_info.verify_info.verify_fail_reason);

	return offset;
}

ssize_t mas_queue_device_sync_read_verify_cp_verify_l4k_store(
	struct request_queue *q, const char *page, size_t count)
{
	ssize_t ret;
	unsigned long val;

	ret = queue_var_store(&val, page, count);
	if (ret < 0)
		return count;

	if (val) {
		tst_cp_verify_l4k = val;
		pr_err("%s, tst_cp_verify_l4k = %d\n", __func__, tst_cp_verify_l4k);
	}

	return count;
}

ssize_t mas_queue_device_sync_read_verify_cp_open_l4k_store(
	struct request_queue *q, const char *page, size_t count)
{
	ssize_t ret;
	unsigned long val;

	ret = queue_var_store(&val, page, count);
	if (ret < 0)
		return count;

	if (val) {
		tst_cp_open_l4k = val;
		pr_err("%s, tst_cp_open_l4k = %d\n", __func__, tst_cp_open_l4k);
	}

	return count;
}

enum bad_block_test_case {
	CASE_1 = 99,
	CASE_2 = 299,
	CASE_3 = 399,
};

ssize_t mas_queue_device_bad_block_notify_regist_store(
	struct request_queue *q, const char *page, size_t count)
{
	ssize_t ret;
	unsigned long val;
	struct blk_dev_lld *lld_fn = mas_blk_get_lld(q);
	struct stor_dev_bad_block_info block_info;

	ret = queue_var_store(&val, page, count);
	if (ret < 0)
		return count;

	if (val == CASE_1) {
		block_info.tlc_total_block_num = tst_tlc_total_block_num;
		block_info.tlc_bad_block_num = tst_tlc_bad_block_num;
		if (lld_fn->unistore_ops.dev_bad_block_notfiy_fn)
			lld_fn->unistore_ops.dev_bad_block_notfiy_fn(block_info,
				lld_fn->unistore_ops.dev_bad_block_notfiy_param_data);
	} else if (val == CASE_2) {
		if (lld_fn && lld_fn->unistore_ops.dev_get_bad_block_info) {
			ret = lld_fn->unistore_ops.dev_get_bad_block_info(q, &block_info);
			if (!ret)
				real_bad_tlc_cnt = block_info.tlc_bad_block_num;
		}
		pr_err("%s, real_bad_tlc_cnt = %u \n", __func__, real_bad_tlc_cnt);
		if (lld_fn && lld_fn->unistore_ops.dev_bad_block_err_inject)
			ret = lld_fn->unistore_ops.dev_bad_block_err_inject(
				q, 0xFF, tst_tlc_bad_block_num);
	} else if (val == CASE_3) {
		if (lld_fn && lld_fn->unistore_ops.dev_bad_block_err_inject)
			ret = lld_fn->unistore_ops.dev_bad_block_err_inject(
				q, 0xFF, real_bad_tlc_cnt);
	}

	return count;
}

ssize_t mas_queue_device_bad_block_total_num_store(
	struct request_queue *q, const char *page, size_t count)
{
	ssize_t ret;
	unsigned long val;

	ret = queue_var_store(&val, page, count);
	if (ret < 0 || ret > 0xFFFF)
		return count;

	if (val) {
		tst_tlc_total_block_num = val;
		pr_err("%s, tst_tlc_total_block_num = %u\n", __func__,
			tst_tlc_total_block_num);
	}

	return count;
}

ssize_t mas_queue_device_bad_block_bad_num_store(
	struct request_queue *q, const char *page, size_t count)
{
	ssize_t ret;
	unsigned long val;

	ret = queue_var_store(&val, page, count);
	if (ret < 0 || ret > 0xFF)
		return count;

	if (val) {
		tst_tlc_bad_block_num = val;
		pr_err("%s, tst_tlc_bad_block_num = %u\n", __func__,
			tst_tlc_bad_block_num);
	}

	return count;
}

int mas_blk_recovery_debug_on(void)
{
	return recovery_debug_on;
}

void mas_blk_recovery_debug_off(void)
{
	recovery_debug_on = 0;
}

ssize_t mas_queue_recovery_debug_on_show(
	struct request_queue *q, char *page)
{
	unsigned long offset;
	offset = snprintf(page, PAGE_SIZE,
		"recovery_debug_on: %d\n", recovery_debug_on);
	return (ssize_t)offset;
}

ssize_t mas_queue_recovery_debug_on_store(
	struct request_queue *q, const char *page, size_t count)
{
	ssize_t ret;
	unsigned long val;

	ret = queue_var_store(&val, page, count);
	if (ret < 0)
		return (ssize_t)count;

	if (val)
		recovery_debug_on = val;
	else
		recovery_debug_on = 0;

	return (ssize_t)count;
}

#endif
