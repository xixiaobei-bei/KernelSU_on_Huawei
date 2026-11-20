/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: mas block unistore
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
#include <trace/iotrace.h>
#include <linux/types.h>
#include <scsi/scsi_host.h>
#include "blk.h"
#include "mas_blk_iosched_interface.h"

struct unistore_lba_info {
	sector_t current_lba;
	sector_t current_pu;
	sector_t update_lba;
	sector_t update_pu;
	sector_t valid_length;
	bool cross_pu;
	unsigned char stream_type;
	bool slc_mode;
};

static bool mas_blk_get_order_inc_unistore(struct request *req,
	unsigned char new_stream_type, struct blk_dev_lld *lld)
{
	if (req->mas_req.fsync_ind) {
		req->mas_req.fsync_ind = false;
		io_trace_unistore_count(UNISTORE_FSYNC_INC_ORDER_CNT, 1);
		return true;
	} else if (lld->last_stream_type == STREAM_TYPE_INVALID) {
		io_trace_unistore_count(UNISTORE_STREAM_INC_ORDER_CNT, 1);
		return true;
	} else if (((new_stream_type) && (!lld->last_stream_type)) ||
			((!new_stream_type) && (lld->last_stream_type))) {
		io_trace_unistore_count(UNISTORE_RPMB_INC_ORDER_CNT, 1);
		return true;
	} else if (((new_stream_type == STREAM_TYPE_RPMB) &&
			(lld->last_stream_type != STREAM_TYPE_RPMB)) ||
			((new_stream_type != STREAM_TYPE_RPMB) &&
			(lld->last_stream_type == STREAM_TYPE_RPMB))) {
		return true;
	} else if (lld->write_curr_cnt >= 0xff) { /* max 255 cnt per order */
		return true;
	}

	return false;
}

void mas_blk_req_get_order_nr_unistore(struct request *req,
	unsigned char new_stream_type, unsigned char *order,
	unsigned char *pre_order_cnt, bool extern_protect)
{
	unsigned long flags = 0;
	bool inc = false;
	struct blk_dev_lld *lld = NULL;

	if (!req_cp(req)) {
		lld = mas_blk_get_lld(req->q);
		if (req->mas_req.protocol_nr) {
			pr_err("old protocol_nr exist! %d\n", req->mas_req.protocol_nr);
			*order = req->mas_req.protocol_nr;
			*pre_order_cnt = req->mas_req.protocol_nr_cnt;
			return;
		}

		inc = mas_blk_get_order_inc_unistore(req, new_stream_type, lld);

		if (!extern_protect)
			spin_lock_irqsave(&lld->write_num_lock, flags);

		if (inc) {
			lld->write_pre_cnt = lld->write_curr_cnt;
			lld->write_curr_cnt = 0;
			lld->write_num++;
			lld->write_num &= 0xff;
			if (unlikely(!lld->write_num))
				lld->write_num++;
			*order = lld->write_num;
			lld->last_stream_type = new_stream_type;
		} else {
			*order = lld->write_num;
		}
		lld->write_curr_cnt++;
		*pre_order_cnt = lld->write_pre_cnt;

		if (!extern_protect)
			spin_unlock_irqrestore(&lld->write_num_lock, flags);

#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
		if (mas_blk_unistore_debug_en() && lld->mas_sec_size && lld->mas_pu_size)
			pr_err("%s, new_stream_type = %u, old_stream_type = %u, "
				"fsync_ind = %d, cur_order = %u, pre_order_cnt = %u, "
				"pu = 0x%llx, offset:0x%llx, lba = 0x%llx - 0x%llx\n",
				__func__, new_stream_type, lld->last_stream_type,
				req->mas_req.fsync_ind, *order, *pre_order_cnt,
				(blk_rq_pos(req) >> SECTION_SECTOR) / (lld->mas_pu_size),
				(blk_rq_pos(req) >> SECTION_SECTOR) % (lld->mas_pu_size),
				(blk_rq_pos(req) >> SECTION_SECTOR) / (lld->mas_sec_size),
				(blk_rq_pos(req) >> SECTION_SECTOR) % (lld->mas_sec_size));
#endif
	}

	req->mas_req.protocol_nr = *order;
	req->mas_req.protocol_nr_cnt = *pre_order_cnt;
}

void mas_blk_partition_remap(struct bio *bio, struct hd_struct *p)
{
	if (!blk_queue_query_unistore_enable(bio->bi_disk->queue))
		return;

	if (unlikely(p->mas_hd.default_stream_id))
		bio->mas_bio.stream_type = p->mas_hd.default_stream_id;
}

static void mas_blk_add_section_list_head(struct blk_dev_lld *lld,
	sector_t expected_lba, unsigned char stream_type)
{
	sector_t expected_section = expected_lba / lld->mas_sec_size;
	struct unistore_section_info *first_section_info = NULL;
	struct unistore_section_info *section_info = kmalloc(sizeof(struct unistore_section_info), GFP_NOIO);

	if (unlikely(!section_info)) {
		section_info = kmalloc(sizeof(struct unistore_section_info), __GFP_NOFAIL);
		if (unlikely(!section_info)) {
			pr_err("%s, kmalloc fail\n", __func__);
#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
			rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
#endif
			return;
		}
	}

	section_info->section_start_lba = lld->mas_sec_size * expected_section;
	section_info->slc_mode = false;
	section_info->section_id = expected_section;
	section_info->rcv_io_complete_flag = false;
	section_info->section_insert_time = ktime_get();

	if (list_empty_careful(&lld->section_list[stream_type - 1])) {
		section_info->next_section_start_lba = 0;
		section_info->next_section_id = 0;
	} else {
		first_section_info = list_first_entry(&lld->section_list[stream_type - 1],
			struct unistore_section_info, section_list);
		section_info->next_section_start_lba = first_section_info->section_start_lba;
		section_info->next_section_id = first_section_info->section_id;
		first_section_info->rcv_io_complete_flag = false;
	}

	list_add(&section_info->section_list, &lld->section_list[stream_type - 1]);
}

static void mas_blk_recovery_with_section_start_lba(struct blk_dev_lld *lld,
	sector_t expected_lba, unsigned char stream_type)
{
	struct unistore_section_info *first_section_info = NULL;
	sector_t expected_section = expected_lba / lld->mas_sec_size;

reupdate:
	if (list_empty_careful(&lld->section_list[stream_type - 1])) {
		mas_blk_update_expected_info(lld, 0, stream_type);
		return;
	}

	first_section_info = list_first_entry(&lld->section_list[stream_type - 1],
		struct unistore_section_info, section_list);
	if ((first_section_info->section_id + 1) == expected_section) {
		if (lld->old_section[stream_type - 1] &&
			(first_section_info->section_id != lld->old_section[stream_type - 1])) {
			lld->old_section[stream_type - 1] = first_section_info->section_id;
			list_del_init(&first_section_info->section_list);
			kfree(first_section_info);
			goto reupdate;
		}

		pr_err("%s no section end, old_section 0x%llx section_id 0x%llx, stream_type %u\n",
				__func__, lld->old_section[stream_type - 1],
				first_section_info->section_id, stream_type);
	}

	mas_blk_update_expected_info(lld, first_section_info->section_start_lba, stream_type);
	first_section_info->rcv_io_complete_flag = false;
}

static void mas_blk_recovery_with_not_section_start_lba(struct blk_dev_lld *lld,
	sector_t expected_lba, unsigned char stream_type)
{
	struct unistore_section_info *first_section_info = NULL;
	sector_t expected_section = expected_lba / lld->mas_sec_size;

	if (!list_empty_careful(&lld->section_list[stream_type - 1])) {
		first_section_info = list_first_entry(&lld->section_list[stream_type - 1],
			struct unistore_section_info, section_list);
		if (expected_section == first_section_info->section_id)
			goto out;

		if (expected_section == first_section_info->next_section_id) {
			lld->old_section[stream_type - 1] = first_section_info->section_id;
			list_del_init(&first_section_info->section_list);
			kfree(first_section_info);
			goto out;
		}
	}

	mas_blk_add_section_list_head(lld, expected_lba, stream_type);

out:
	if (list_empty_careful(&lld->section_list[stream_type - 1]))
		return;

	first_section_info = list_first_entry(&lld->section_list[stream_type - 1],
			struct unistore_section_info, section_list);
	first_section_info->rcv_io_complete_flag = true;
}

void mas_blk_recovery_update_section_list(struct blk_dev_lld *lld,
	sector_t expected_lba, unsigned char stream_type)
{
	if (expected_lba % lld->mas_sec_size)
		mas_blk_recovery_with_not_section_start_lba(lld, expected_lba, stream_type);
	else
		mas_blk_recovery_with_section_start_lba(lld, expected_lba, stream_type);

	lld->old_section[stream_type - 1] = 0;
}

bool mas_blk_is_section_ready(struct request_queue *q, struct bio *bio)
{
	struct unistore_section_info *section_info = NULL;
	struct blk_dev_lld *lld = mas_blk_get_lld(q);
	unsigned char stream_type;
	sector_t req_lba;
	unsigned long flags;
	sector_t current_section;
	bool ready_flag = false;

	if (!bio || !blk_queue_query_unistore_enable(q) ||
		(bio_op(bio) != REQ_OP_WRITE))
		return true;

	if (!lld || !lld->mas_sec_size || !lld->mas_pu_size)
		return false;

	stream_type = bio->mas_bio.stream_type;
	if (!mas_blk_is_order_stream(stream_type))
		return true;

	req_lba = bio->bi_iter.bi_sector >> SECTION_SECTOR;
	current_section = req_lba / lld->mas_sec_size;
	spin_lock_irqsave(&lld->expected_lba_lock[stream_type - 1], flags);
	if (list_empty_careful(&lld->section_list[stream_type - 1])) {
		ready_flag = true;
		mas_blk_add_section_list_head(lld, current_section * lld->mas_sec_size, stream_type);
	} else {
		section_info = list_first_entry(&lld->section_list[stream_type - 1],
			struct unistore_section_info, section_list);
		ready_flag = (current_section == section_info->section_id) ||
			(section_info->rcv_io_complete_flag &&
				(current_section == section_info->next_section_id));
	}
	spin_unlock_irqrestore(&lld->expected_lba_lock[stream_type - 1], flags);
	return ready_flag;
}

static void mas_blk_get_lba_info(struct blk_dev_lld *lld,
	struct unistore_lba_info *lba_info, bool slc_mode,
	sector_t current_lba, sector_t update_lba,
	sector_t origin_length, unsigned char stream_type)
{
	lba_info->current_lba = current_lba;
	lba_info->update_lba = update_lba;
	lba_info->current_pu = current_lba / lld->mas_pu_size;
	lba_info->update_pu = update_lba / lld->mas_pu_size;
	lba_info->valid_length = (lba_info->update_pu > lba_info->current_pu) ?
		((lba_info->current_pu + 1) * lld->mas_pu_size - current_lba) : origin_length;
	lba_info->cross_pu = (origin_length >= lld->mas_pu_size) ||
			(origin_length > lba_info->valid_length);
	lba_info->stream_type = stream_type;
	lba_info->slc_mode = slc_mode;

	if (!lba_info->cross_pu)
		return;

	if (update_lba % lld->mas_pu_size)
		lba_info->current_lba = update_lba - update_lba % lld->mas_pu_size;
	else
		lba_info->current_lba = update_lba - lld->mas_pu_size;
	lba_info->current_pu = lba_info->current_lba / lld->mas_pu_size;
	lba_info->valid_length = lba_info->update_lba - lba_info->current_lba;
}

bool mas_blk_enable_disorder_stream(unsigned char stream_type)
{
	return (stream_type == BLK_STREAM_HOT_DATA) && mas_blk_enable_disorder();
}

bool mas_blk_expected_lba_pu(struct blk_dev_lld *lld,
	unsigned char stream, sector_t req_lba)
{
	if (mas_blk_enable_disorder_stream(stream))
		return (lld->expected_pu[stream - 1] == (req_lba / lld->mas_pu_size));
	else
		return (lld->expected_lba[stream - 1] == req_lba);
}

static void mas_blk_update_lba_info(struct blk_dev_lld *lld,
	sector_t update_lba, unsigned char stream_type)
{
	lld->expected_lba[stream_type - 1] = update_lba;
	lld->expected_refresh_time[stream_type - 1] = ktime_get();
}

static void mas_blk_update_new_pu_info(struct blk_dev_lld *lld,
	sector_t update_lba, unsigned char stream_type)
{
	lld->expected_pu[stream_type - 1] = update_lba / lld->mas_pu_size;
	lld->current_pu_size[stream_type - 1] = update_lba % lld->mas_pu_size;
	lld->expected_refresh_time[stream_type - 1] = ktime_get();
}

void mas_blk_update_expected_info(struct blk_dev_lld *lld,
	sector_t update_lba, unsigned char stream_type)
{
	if (mas_blk_enable_disorder_stream(stream_type))
		mas_blk_update_new_pu_info(lld, update_lba, stream_type);
	else
		mas_blk_update_lba_info(lld, update_lba, stream_type);
}

static void mas_blk_update_cur_pu_info(struct blk_dev_lld *lld,
	sector_t current_lba, sector_t valid_length, unsigned char stream_type)
{
	lld->expected_pu[stream_type - 1] = current_lba / lld->mas_pu_size;
	lld->current_pu_size[stream_type - 1] = valid_length;
	lld->expected_refresh_time[stream_type - 1] = ktime_get();
}

static void mas_blk_not_update_section(struct blk_dev_lld *lld,
	struct unistore_section_info *section_info, struct unistore_lba_info *lba_info)
{
	sector_t expected_section =
		lld->expected_lba[lba_info->stream_type - 1] / (lld->mas_sec_size);

	section_info->rcv_io_complete_flag = true;
	if ((expected_section == section_info->section_id) &&
		(lba_info->update_lba < lld->expected_lba[lba_info->stream_type - 1]))
			return;

	mas_blk_update_lba_info(lld, lba_info->update_lba, lba_info->stream_type);
}

static void mas_blk_update_section(struct blk_dev_lld *lld,
	struct unistore_section_info *section_info, struct unistore_lba_info *lba_info)
{
	struct unistore_section_info *first_section_info = NULL;
	sector_t update_section = lba_info->update_lba / (lld->mas_sec_size);

	lld->old_section[lba_info->stream_type - 1] = section_info->section_id;
	list_del_init(&section_info->section_list);
	kfree(section_info);

	if (list_empty_careful(&lld->section_list[lba_info->stream_type - 1])) {
		mas_blk_update_lba_info(lld, 0, lba_info->stream_type);
	} else {
		first_section_info = list_first_entry(
				&lld->section_list[lba_info->stream_type - 1],
				struct unistore_section_info, section_list);

		if (update_section == first_section_info->section_id) {
			mas_blk_update_lba_info(lld,
				lba_info->update_lba, lba_info->stream_type);
			if (lba_info->update_lba > first_section_info->section_start_lba)
				first_section_info->rcv_io_complete_flag = true;
		} else {
			mas_blk_update_lba_info(lld,
				first_section_info->section_start_lba, lba_info->stream_type);
		}
	}
}

static void mas_blk_update_new_section_pu(struct blk_dev_lld *lld,
	struct unistore_section_info *section_info, struct unistore_lba_info *lba_info)
{
	struct unistore_section_info *first_section_info = NULL;

	lld->old_section[lba_info->stream_type - 1] = section_info->section_id;
	list_del_init(&section_info->section_list);
	kfree(section_info);

	first_section_info = list_first_entry(
			&lld->section_list[lba_info->stream_type - 1],
			struct unistore_section_info, section_list);
	first_section_info->rcv_io_complete_flag = true;

	if (lba_info->cross_pu)
		mas_blk_update_new_pu_info(lld, lba_info->update_lba, lba_info->stream_type);
	else
		mas_blk_update_cur_pu_info(lld, lba_info->current_lba,
			lba_info->valid_length, lba_info->stream_type);
}

static bool mas_blk_is_section_end(struct blk_dev_lld *lld,
	struct unistore_section_info *section_info, struct unistore_lba_info *lba_info)
{
	if (lld->current_pu_size[lba_info->stream_type - 1])
		return false;

	if (!(lld->expected_pu[lba_info->stream_type - 1] * lld->mas_pu_size % lld->mas_sec_size))
		return true;

	if (!(lld->expected_pu[lba_info->stream_type - 1] * lld->mas_pu_size %
		(lld->mas_sec_size / 3)) && lba_info->slc_mode)
		return true; /* tlc : slc = 3 */

	return false;
}

static void mas_blk_update_section_end_pu(struct blk_dev_lld *lld,
	struct unistore_section_info *section_info, struct unistore_lba_info *lba_info)
{
	struct unistore_section_info *first_section_info = NULL;

	if (!mas_blk_is_section_end(lld, section_info, lba_info))
		return;

	lld->old_section[lba_info->stream_type - 1] = section_info->section_id;
	list_del_init(&section_info->section_list);
	kfree(section_info);

	if (list_empty_careful(&lld->section_list[lba_info->stream_type - 1])) {
		mas_blk_update_new_pu_info(lld, 0, lba_info->stream_type);
	} else {
		first_section_info = list_first_entry(
				&lld->section_list[lba_info->stream_type - 1],
				struct unistore_section_info, section_list);
		mas_blk_update_new_pu_info(lld,
			first_section_info->section_start_lba, lba_info->stream_type);
	}
}

static void mas_blk_update_same_pu(struct blk_dev_lld *lld,
	struct unistore_section_info *section_info, struct unistore_lba_info *lba_info)
{
	lld->current_pu_size[lba_info->stream_type - 1] += lba_info->valid_length;
	if (lld->current_pu_size[lba_info->stream_type - 1] != lld->mas_pu_size)
		return;

	mas_blk_update_new_pu_info(lld, (lba_info->current_pu + 1) * lld->mas_pu_size,
		lba_info->stream_type);
	mas_blk_update_section_end_pu(lld, section_info, lba_info);
}

static void mas_blk_update_subsequent_pu(struct blk_dev_lld *lld,
	struct unistore_section_info *section_info, struct unistore_lba_info *lba_info)
{
	if (lba_info->cross_pu) {
		mas_blk_update_new_pu_info(lld, lba_info->update_lba, lba_info->stream_type);
		mas_blk_update_section_end_pu(lld, section_info, lba_info);
	} else {
		mas_blk_update_cur_pu_info(lld, lba_info->current_lba,
			lba_info->valid_length, lba_info->stream_type);
	}
}

static void mas_blk_update_same_section_pu(struct blk_dev_lld *lld,
	struct unistore_section_info *section_info, struct unistore_lba_info *lba_info)
{
	if (lld->expected_pu[lba_info->stream_type - 1] > lba_info->current_pu)
		return;

	section_info->rcv_io_complete_flag = true;
	if (lld->expected_pu[lba_info->stream_type - 1] == lba_info->current_pu)
		mas_blk_update_same_pu(lld, section_info, lba_info);
	else
		mas_blk_update_subsequent_pu(lld, section_info, lba_info);
}

static void mas_blk_update_expected_pu(struct blk_dev_lld *lld,
	struct unistore_section_info *section_info, struct unistore_lba_info *lba_info)
{
	sector_t current_section = lba_info->current_lba / (lld->mas_sec_size);

	if (current_section == section_info->section_id)
		mas_blk_update_same_section_pu(lld, section_info, lba_info);
	else
		mas_blk_update_new_section_pu(lld, section_info, lba_info);
}

static void mas_blk_update_expected_lba(struct request *req, unsigned int nr_bytes)
{
	struct blk_dev_lld *lld = mas_blk_get_lld(req->q);
	sector_t current_lba = blk_rq_pos(req) >> SECTION_SECTOR;
	sector_t update_lba;
	sector_t current_section;
	sector_t update_section;
	unsigned char stream_type = req->mas_req.stream_type;
	unsigned long flags = 0;
	struct unistore_section_info *section_info = NULL;
	struct unistore_lba_info lba_info;

	if ((req_op(req) != REQ_OP_WRITE) ||
		!mas_blk_is_order_stream(stream_type) ||
		!lld->mas_sec_size || !lld->mas_pu_size)
		return;

	current_section = current_lba / lld->mas_sec_size;
	update_lba = current_lba + ((nr_bytes >> SECTOR_BYTE) >> SECTION_SECTOR);
	if (update_lba % (lld->mas_sec_size) == (lld->mas_sec_size / 3) && req->mas_req.slc_mode)
		update_lba += ((lld->mas_sec_size) - (lld->mas_sec_size / 3)); /* tlc : slc = 3 */
	update_section = update_lba / (lld->mas_sec_size);

	spin_lock_irqsave(&lld->expected_lba_lock[stream_type - 1], flags);
	if (list_empty_careful(&lld->section_list[stream_type - 1])) {
		mas_blk_update_expected_info(lld, 0, stream_type);
		goto out;
	}

	section_info = list_first_entry(&lld->section_list[stream_type - 1],
			struct unistore_section_info, section_list);
	if ((current_section != section_info->section_id) &&
		(current_section != section_info->next_section_id))
		goto out;

	mas_blk_get_lba_info(lld, &lba_info, req->mas_req.slc_mode, current_lba, update_lba,
		((nr_bytes >> SECTOR_BYTE) >> SECTION_SECTOR), stream_type);

	if (mas_blk_enable_disorder_stream(stream_type)) {
		mas_blk_update_expected_pu(lld, section_info, &lba_info);
		goto out;
	}

	if (update_section == section_info->section_id)
		mas_blk_not_update_section(lld, section_info, &lba_info);
	else
		mas_blk_update_section(lld, section_info, &lba_info);

out:
	spin_unlock_irqrestore(&lld->expected_lba_lock[stream_type - 1], flags);
}

bool mas_blk_match_expected_lba(struct request_queue *q, struct bio *bio)
{
	unsigned long flags = 0;
	struct blk_dev_lld *lld = NULL;
	sector_t current_lba;
	unsigned char stream_type;
	bool ret = false;

	if (!q || !bio)
		return false;

	lld = mas_blk_get_lld(q);
	if (!(lld->features & BLK_LLD_UFS_UNISTORE_EN) || !lld->mas_pu_size)
		return false;

	current_lba = bio->bi_iter.bi_sector >> SECTION_SECTOR;
	stream_type = bio->mas_bio.stream_type;

	if (!mas_blk_is_order_stream(stream_type))
		return false;

	spin_lock_irqsave(&lld->expected_lba_lock[stream_type - 1], flags);
	ret = mas_blk_expected_lba_pu(lld, stream_type, current_lba);
	spin_unlock_irqrestore(&lld->expected_lba_lock[stream_type - 1], flags);

	return ret;
}

void mas_blk_set_lba_flag(struct request *rq, struct blk_dev_lld *lld)
{
	unsigned long flags = 0;
	unsigned char stream_type = rq->mas_req.stream_type;
	sector_t req_lba = blk_rq_pos(rq) >> SECTION_SECTOR;

	if (!(lld->features & BLK_LLD_UFS_UNISTORE_EN) || !lld->mas_pu_size)
		return;

	if (req_op(rq) != REQ_OP_WRITE)
		return;

	if (!mas_blk_is_order_stream(stream_type))
		return;

	spin_lock_irqsave(&lld->expected_lba_lock[stream_type - 1], flags);
	if (mas_blk_expected_lba_pu(lld, stream_type, req_lba))
		rq->mas_cmd_flags |= MAS_REQ_LBA;
	spin_unlock_irqrestore(&lld->expected_lba_lock[stream_type - 1], flags);
}

static atomic_t buf_pages_anon;
static atomic_t buf_pages_non_anon;
#define MAX_WARNING_PAGES 10000
int mas_blk_get_recovery_pages(bool page_anon)
{
	return page_anon ? atomic_read(&buf_pages_anon) : atomic_read(&buf_pages_non_anon);
}

void mas_blk_recovery_pages_add(struct page *page)
{
	if (PageAnon(page)) {
		atomic_inc(&buf_pages_anon);
		return;
	}

	SetPageCached(page);
	atomic_inc(&buf_pages_non_anon);
}

void mas_blk_recovery_pages_sub(struct page *page)
{
	if (PageAnon(page)) {
		atomic_dec(&buf_pages_anon);
		return;
	}

	ClearPageCached(page);
	atomic_dec(&buf_pages_non_anon);
}

static void mas_blk_put_page_in_buf_list(struct bio* bio,
	unsigned int del_page_cnt)
{
	int i;
	struct bio_vec *bvec = bio->bi_io_vec + bio->bi_vcnt - bio->recovery_page_cnt;

	for (i = 0; i < del_page_cnt; ++i) {
		ClearPageCached(bvec->bv_page);
		put_page(bvec->bv_page);
		bvec++;
	}
}

static void mas_blk_get_page_in_buf_list(struct bio* bio)
{
	int i;
	struct bio_vec *bvec = bio->bi_io_vec + bio->bi_vcnt - bio->max_recovery_num;

	if (!PageAnon(bvec->bv_page)) {
		bio->non_anon_page_flag = true;
		atomic_add(bio->max_recovery_num, &buf_pages_non_anon);
	} else {
		bio->non_anon_page_flag = false;
		atomic_add(bio->max_recovery_num, &buf_pages_anon);
	}

	for (i = 0; i < bio->max_recovery_num; ++i) {
		get_page(bvec->bv_page);
		if (bio->non_anon_page_flag)
			SetPageCached(bvec->bv_page);
		bvec++;
	}
}

static bool mas_blk_update_bio_page(struct bio *pos,
	struct page *page, struct page *cached_page)
{
	unsigned int i;
	struct bio_vec *bvec = pos->bi_io_vec + pos->bi_vcnt - pos->recovery_page_cnt;

	if (!pos->non_anon_page_flag)
		return false;

	for (i = 0; i < pos->recovery_page_cnt; ++i) {
		if (bvec->bv_page == page) {
			bvec->bv_page = cached_page;
			SetPageCached(cached_page);
			mas_blk_recovery_pages_sub(page);
			put_page(page);
			return true;
		}

		bvec++;
	}

	return false;
}

int mas_blk_update_buf_bio_page(struct block_device *bdev,
	struct page *page, struct page *cached_page)
{
	int i;
	unsigned long flag;
	struct request_queue *q = NULL;
	struct blk_dev_lld *lld = NULL;
	struct bio *pos = NULL;

	q = bdev_get_queue(bdev);
	if (!blk_queue_query_unistore_enable(q))
		return -EFAULT;

	lld = mas_blk_get_lld(q);
	if (!lld)
		return -EPERM;

	down_read(&lld->recovery_rwsem);
	for (i = BLK_ORDER_STREAM_NUM; i >= 1; i--) {
		spin_lock_irqsave(&lld->buf_bio_list_lock[i], flag);
		if (list_empty_careful(&lld->buf_bio_list[i])) {
			spin_unlock_irqrestore(&lld->buf_bio_list_lock[i], flag);
			continue;
		}

		list_for_each_entry(pos, &lld->buf_bio_list[i], buf_bio_list_node) {
			if (mas_blk_update_bio_page(pos, page, cached_page)) {
				atomic_inc(&lld->replaced_page_cnt[i]);
				spin_unlock_irqrestore(&lld->buf_bio_list_lock[i], flag);
				up_read(&lld->recovery_rwsem);
				return 0;
			}
		}
		spin_unlock_irqrestore(&lld->buf_bio_list_lock[i], flag);
	}
	up_read(&lld->recovery_rwsem);
	return -1;
}

#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
#define BIO_BUF_LIST_MAX_PAGE mas_blk_get_max_recovery_num()
#define BIO_BUF_LIST_DEL_NUM mas_blk_get_max_del_num()
#else
#define BIO_BUF_LIST_MAX_PAGE 72
#define BIO_BUF_LIST_DEL_NUM 2
#endif

static void mas_blk_del_bio_in_buf_list(struct blk_dev_lld *lld,
	struct bio *pos, unsigned char stream_type, unsigned int del_page_cnt)
{
	if (del_page_cnt >= pos->recovery_page_cnt) {
		list_del_init(&pos->buf_bio_list_node);
		if (pos->non_anon_page_flag)
			atomic_sub(pos->max_recovery_num, &buf_pages_non_anon);
		else
			atomic_sub(pos->max_recovery_num, &buf_pages_anon);

		mas_blk_put_page_in_buf_list(pos, pos->recovery_page_cnt);
		lld->buf_bio_size[stream_type] -= pos->ori_bi_iter.bi_size;
		lld->buf_page_num[stream_type] -= pos->recovery_page_cnt;
		lld->buf_bio_num[stream_type]--;
		pos->buf_bio = false;
		bio_put(pos);
		return;
	}

	mas_blk_put_page_in_buf_list(pos, del_page_cnt);
	pos->recovery_page_cnt -= del_page_cnt;
	lld->buf_page_num[stream_type] -= del_page_cnt;
}

static bool mas_blk_bio_after(struct blk_dev_lld *lld,
	struct bio *bio1, struct bio *bio2, unsigned char stream_type)
{
	unsigned long flags;
	sector_t bio1_section;
	sector_t bio2_section;
	sector_t cur_section;
	bool old_section = false;

	bio1_section = (bio1->bi_iter.bi_sector >> SECTION_SECTOR) / lld->mas_sec_size;
	bio2_section = (bio2->ori_bi_iter.bi_sector >> SECTION_SECTOR) / lld->mas_sec_size;
	if (bio1_section == bio2_section)
		return bio1->bi_iter.bi_sector > bio2->ori_bi_iter.bi_sector ? true : false;

	spin_lock_irqsave(&lld->expected_lba_lock[stream_type - 1], flags);
	cur_section = (!mas_blk_enable_disorder_stream(stream_type) ?
		(lld->expected_lba[stream_type - 1] / lld->mas_sec_size) :
		(lld->expected_pu[stream_type - 1] * lld->mas_pu_size / lld->mas_sec_size));
	old_section = (bio1_section != cur_section ? true : false);
	spin_unlock_irqrestore(&lld->expected_lba_lock[stream_type - 1], flags);

	return !old_section;
}

static void mas_blk_order_insert_buf_list(struct bio *bio,
	struct blk_dev_lld *lld, unsigned char stream_type)
{
	struct list_head *target_list = NULL;
	struct bio *pos = NULL;

	target_list = &lld->buf_bio_list[stream_type];

#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
	if (mas_blk_unistore_debug_en())
		pr_err("recovery_order, stream: %d, bio_nr: %u, "
			"insert_bio: 0x%llx - 0x%llx, len: %u, done: %u\n",
			stream_type, bio->bio_nr,
			(bio->bi_iter.bi_sector >> SECTION_SECTOR) /
				(lld->mas_sec_size),
			(bio->bi_iter.bi_sector >> SECTION_SECTOR) %
				(lld->mas_sec_size),
			bio->bi_iter.bi_size / BLKSIZE,
			bio->bi_iter.bi_done / BLKSIZE);
#endif

	if (!list_empty_careful(target_list)) {
		list_for_each_entry_reverse(pos, target_list, buf_bio_list_node) {
			if (!pos->bio_nr)
				continue;
			if (mas_blk_bio_after(lld, bio, pos, stream_type)) {
				list_add(&bio->buf_bio_list_node,
					&pos->buf_bio_list_node);
				break;
			}
		}
		if (&pos->buf_bio_list_node == target_list)
			list_add(&bio->buf_bio_list_node,
				&pos->buf_bio_list_node);
	} else {
		list_add(&bio->buf_bio_list_node, target_list);
	}
}

static void mas_blk_buf_list_update(struct blk_dev_lld *lld,
	struct bio *bio, unsigned char stream_type)
{
	struct bio *pos = NULL;
	unsigned int del_page_cnt;
	unsigned int del_cnt = 0;

	mas_blk_order_insert_buf_list(bio, lld, stream_type);
	bio_get(bio);
	bio->buf_bio = true;
	bio->max_recovery_num = (bio->bi_vcnt < BIO_BUF_LIST_MAX_PAGE) ?
		bio->bi_vcnt : BIO_BUF_LIST_MAX_PAGE;
	bio->recovery_page_cnt = bio->max_recovery_num;
	memcpy(&(bio->ori_bi_iter), &(bio->bi_iter), sizeof(struct bvec_iter));
	mas_blk_get_page_in_buf_list(bio);
	lld->buf_bio_size[stream_type] += bio->ori_bi_iter.bi_size;
	lld->buf_page_num[stream_type] += bio->max_recovery_num;
	lld->buf_bio_num[stream_type]++;

#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
	if (mas_blk_unistore_debug_en())
		pr_err("recovery_add, steam: %d, io_num: %u, size: %u, page_num: %u, "
			"add_bio: 0x%llx - 0x%llx, len: %u, done: %u\n",
			stream_type,
			lld->buf_bio_num[stream_type],
			lld->buf_bio_size[stream_type] / BLKSIZE,
			lld->buf_page_num[stream_type],
			(bio->bi_iter.bi_sector >> SECTION_SECTOR) /
				(lld->mas_sec_size),
			(bio->bi_iter.bi_sector >> SECTION_SECTOR) %
				(lld->mas_sec_size),
			bio->bi_iter.bi_size / BLKSIZE,
			bio->bi_iter.bi_done / BLKSIZE);
#endif

	while (lld->buf_page_num[stream_type] > BIO_BUF_LIST_MAX_PAGE) {
		pos = list_first_entry(&lld->buf_bio_list[stream_type],
					struct bio, buf_bio_list_node);

		del_page_cnt = lld->buf_page_num[stream_type] - BIO_BUF_LIST_MAX_PAGE;

#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
		if (mas_blk_unistore_debug_en())
			pr_err("recovery_del, del_bio: 0x%llx - 0x%llx, len: %u, done: %u\n",
				(pos->ori_bi_iter.bi_sector >> SECTION_SECTOR) /
					(lld->mas_sec_size),
				(pos->ori_bi_iter.bi_sector >> SECTION_SECTOR) %
					(lld->mas_sec_size),
				pos->ori_bi_iter.bi_size / BLKSIZE,
				pos->ori_bi_iter.bi_done / BLKSIZE);
#endif
		mas_blk_del_bio_in_buf_list(lld, pos, stream_type, del_page_cnt);
		del_cnt++;
		if (del_cnt >= BIO_BUF_LIST_DEL_NUM)
			break;
	}
}

static void mas_blk_add_bio_to_buf_list(struct request *req)
{
	unsigned long flags;
	unsigned char stream_type;
	struct blk_dev_lld *lld = NULL;
	struct bio *bio = NULL;

	if (!mas_blk_is_order_stream(req->mas_req.stream_type))
		return;

	if (!(req->bio) || !(op_is_write(bio_op(req->bio))))
		return;

	if (req->bio->buf_bio)
		return;

	bio = req->bio;
	bio->bio_nr = req->mas_req.protocol_nr;
	stream_type = req->bio->mas_bio.stream_type;
	lld = mas_blk_get_lld(req->q);
	if (!lld || !lld->mas_sec_size)
		return;

	spin_lock_irqsave(&lld->buf_bio_list_lock[stream_type], flags);
	mas_blk_buf_list_update(lld, bio, stream_type);
	spin_unlock_irqrestore(&lld->buf_bio_list_lock[stream_type], flags);
}

static inline void mas_blk_buf_list_end_bio(struct bio* bio)
{
	return;
}

static bool mas_blk_bio_in_section_list(
	struct blk_dev_lld *lld, struct bio *bio, sector_t bio_section)
{
	unsigned long flags;
	struct unistore_section_info *section_info = NULL;

	spin_lock_irqsave(
		&lld->expected_lba_lock[bio->mas_bio.stream_type - 1], flags);
	list_for_each_entry(section_info,
		&lld->section_list[bio->mas_bio.stream_type - 1], section_list) {
		if ((section_info->section_start_lba / (lld->mas_sec_size)) == bio_section) {
			spin_unlock_irqrestore(
				&lld->expected_lba_lock[bio->mas_bio.stream_type - 1], flags);
			return true;
		}
	}
	spin_unlock_irqrestore(
		&lld->expected_lba_lock[bio->mas_bio.stream_type - 1], flags);

	return false;
}

bool mas_blk_bio_need_dispatch(struct bio* bio,
	struct request_queue *q, struct stor_dev_pwron_info* stor_info)
{
	sector_t bio_lba;
	sector_t bio_len;
	sector_t bio_section;
	sector_t device_lba;
	sector_t device_section;
	struct blk_dev_lld *lld = NULL;

	if (!q || !bio || !stor_info)
		return false;

	if (!mas_blk_is_order_stream(bio->mas_bio.stream_type))
		return false;

	if (bio->rec_bio_list_node.prev != bio->rec_bio_list_node.next)
		return false;

	lld = mas_blk_get_lld(q);
	if (!lld || !lld->mas_sec_size)
		return false;

	bio_lba = bio->ori_bi_iter.bi_sector >> SECTION_SECTOR;
	bio_len = bio->ori_bi_iter.bi_size / BLKSIZE;
	bio_section = bio_lba / (lld->mas_sec_size);

	device_lba = stor_info->dev_stream_addr[bio->mas_bio.stream_type];
	device_section = device_lba / (lld->mas_sec_size);

	if (bio_section == device_section) {
		/* bio after device open ptr */
		if (bio_lba >= device_lba)
			goto need_dispatch;

		/* bio over device open ptr */
		if ((bio_lba + bio_len) > device_lba) {
#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
			pr_err("%s, bio_len: %u, bio_lba: 0x%llx - 0x%llx\n",
					__func__, bio_len,
					bio_lba / lld->mas_sec_size,
					bio_lba % lld->mas_sec_size);
#endif
			bio_advance_iter(bio, &bio->ori_bi_iter,
					(device_lba - bio_lba) * BLKSIZE);
			lld->buf_bio_size[bio->mas_bio.stream_type] -=
					(device_lba - bio_lba) * BLKSIZE;
			goto need_dispatch;
		}
	} else {
		/* bio after section of device open ptr */
		if (mas_blk_bio_in_section_list(lld, bio, bio_section))
			goto need_dispatch;
	}

	return false;

need_dispatch:
	memcpy(&bio->bi_iter, &bio->ori_bi_iter, sizeof(struct bvec_iter));
	bio->bi_end_io = mas_blk_buf_list_end_bio;
	return true;
}

void mas_blk_req_update_unistore(
	struct request *req, blk_status_t error, unsigned int nr_bytes)
{
	if (!blk_queue_query_unistore_enable(req->q))
		return;

	if (error || !nr_bytes)
		return;

	mas_blk_update_expected_lba(req, nr_bytes);
	mas_blk_add_bio_to_buf_list(req);
}

static void mas_blk_bad_block_notify_fn(struct Scsi_Host *host,
	struct stor_dev_bad_block_info *bad_block_info)
{
	struct blk_dev_lld *lld = &(host->tag_set.lld_func);

	if (!lld)
		return;

	lld->bad_block_info = *bad_block_info;
	if (!atomic_cmpxchg(&lld->bad_block_atomic, 0, 1))
		schedule_work(&lld->bad_block_work);
}

static void mas_blk_bad_block_notify_handler(struct work_struct *work)
{
	struct blk_dev_lld *lld = container_of(
		work, struct blk_dev_lld, bad_block_work);

	if (!lld || !lld->unistore_ops.dev_bad_block_notfiy_fn)
		return;

	lld->unistore_ops.dev_bad_block_notfiy_fn(lld->bad_block_info,
		lld->unistore_ops.dev_bad_block_notfiy_param_data);

	atomic_set(&lld->bad_block_atomic, 0);
}

static void mas_blk_bad_block_notify_work_init(struct request_queue *q)
{
	struct blk_dev_lld *lld = mas_blk_get_lld(q);

	if (!lld || !lld->unistore_ops.dev_bad_block_notify_register)
		return;

	atomic_set(&lld->bad_block_atomic, 0);
	INIT_WORK(&lld->bad_block_work, mas_blk_bad_block_notify_handler);
	lld->unistore_ops.dev_bad_block_notify_register(q, mas_blk_bad_block_notify_fn);
}

void mas_blk_request_init_from_bio_unistore(
	struct request *req, struct bio *bio)
{
	req->mas_req.stream_type = bio->mas_bio.stream_type;
	req->mas_req.slc_mode = bio->mas_bio.slc_mode;
	req->mas_req.cp_tag = bio->mas_bio.cp_tag;
	req->mas_req.data_ino = bio->mas_bio.data_ino;
	req->mas_req.data_idx = bio->mas_bio.data_idx;
	req->mas_req.fsync_ind = bio->mas_bio.fsync_ind;
	req->mas_req.fg_io = bio->mas_bio.fg_io;
}

void mas_blk_request_init_unistore(struct request *req)
{
	req->mas_req.stream_type = STREAM_TYPE_INVALID;
	req->mas_req.slc_mode = false;
	req->mas_req.cp_tag = 0;
	req->mas_req.data_ino = 0;
	req->mas_req.data_idx = 0;
	req->mas_req.fsync_ind = false;
	req->mas_req.fg_io = false;
}

void mas_blk_order_info_reset(struct blk_dev_lld *lld)
{
	if (!(lld->features & BLK_LLD_UFS_UNISTORE_EN))
		return;

	lld->last_stream_type = STREAM_TYPE_INVALID;
	lld->fsync_ind = false;
	lld->write_pre_cnt = 0;
	lld->write_curr_cnt = 0;
}

void mas_blk_dev_lld_init_unistore(struct blk_dev_lld *blk_lld)
{
	unsigned int i;

	memset(blk_lld->lock_map, 0, NR_CPUS);

	blk_lld->unistore_ops.dev_bad_block_notfiy_fn = NULL;
	blk_lld->unistore_ops.dev_bad_block_notfiy_param_data = NULL;
	blk_lld->last_stream_type = STREAM_TYPE_INVALID;
	blk_lld->fsync_ind = false;
	blk_lld->mas_sec_size = 0;
	blk_lld->mas_pu_size = 0;
	blk_lld->max_recovery_size = 0;
	spin_lock_init(&blk_lld->fsync_ind_lock);
	for (i = 0; i < BLK_ORDER_STREAM_NUM; i++) {
		spin_lock_init(&blk_lld->expected_lba_lock[i]);
		blk_lld->expected_lba[i] = 0;
		blk_lld->expected_pu[i] = 0;
		blk_lld->current_pu_size[i] = 0;
		blk_lld->expected_refresh_time[i] = 0;
		blk_lld->old_section[i] = 0;
		INIT_LIST_HEAD(&blk_lld->section_list[i]);
	}
	for (i = 0; i <= BLK_ORDER_STREAM_NUM; i++) {
		INIT_LIST_HEAD(&blk_lld->buf_bio_list[i]);
		spin_lock_init(&blk_lld->buf_bio_list_lock[i]);
		blk_lld->buf_bio_size[i] = 0;
		blk_lld->buf_bio_num[i] = 0;
		atomic_set(&blk_lld->replaced_page_cnt[i], 0);
	}
	atomic_set(&blk_lld->reset_cnt, 0);
	atomic_set(&blk_lld->recovery_flag, 0);
	atomic_set(&blk_lld->recovery_pwron_flag, 0);
	atomic_set(&blk_lld->recovery_pwron_inprocess_flag, 0);
	init_rwsem(&blk_lld->recovery_rwsem);
	mutex_init(&blk_lld->recovery_mutex);

	blk_lld->write_curr_cnt = 0;
	blk_lld->write_pre_cnt = 0;

	atomic_set(&buf_pages_anon, 0);
	atomic_set(&buf_pages_non_anon, 0);

	blk_lld->features &= ~BLK_LLD_UFS_UNISTORE_EN;
}

void mas_blk_bio_set_opf_unistore(struct bio *bio)
{
	struct blk_dev_lld *lld = NULL;
	unsigned long flags = 0;

	if (!blk_queue_query_unistore_enable(bio->bi_disk->queue))
		return;

	bio->mas_bio.fsync_ind = false;
	if (op_is_write(bio_op(bio))) {
		lld = mas_blk_get_lld(bio->bi_disk->queue);
		spin_lock_irqsave(&lld->fsync_ind_lock, flags);
		if (lld && lld->fsync_ind) {
			bio->mas_bio.fsync_ind = true;
			lld->fsync_ind = false;
		}
		spin_unlock_irqrestore(&lld->fsync_ind_lock, flags);
	}

	bio->mas_bio.fg_io = false;
	if ((bio->bi_opf & (REQ_FG | REQ_VIP)) &&
		!(bio->bi_opf & (REQ_META | REQ_PRIO)))
		bio->mas_bio.fg_io = true;

	bio->buf_bio = false;
	bio->mas_bio.bi_opf = bio->bi_opf;
	if (op_is_write(bio_op(bio))) {
		bio->bi_opf &= ~REQ_META;
		bio->bi_opf &= ~REQ_FG;
		bio->bi_opf &= ~REQ_VIP;
		if (mas_blk_is_order_stream(bio->mas_bio.stream_type))
			bio->bi_opf &= ~REQ_PREFLUSH;
		bio->bi_opf |= REQ_SYNC;
	}
}

struct bio* mas_blk_bio_segment_bytes_split(
	struct bio *bio, struct bio_set *bs, unsigned int bytes,
	unsigned front_seg_size, unsigned seg_size)
{
	struct bio *new = bio_split(bio, bytes >> SECTOR_BYTE, GFP_NOIO, bs);
	if (new)
		bio = new;

	bio->bi_seg_front_size = front_seg_size;
	if (seg_size > bio->bi_seg_back_size)
		bio->bi_seg_back_size = seg_size;

	return new;
}

unsigned int mas_blk_bio_get_residual_byte(
	struct request_queue *q, struct bvec_iter iter)
{
	unsigned int mas_sec_size = 0;
	struct blk_dev_lld *lld = mas_blk_get_lld(q);
	if (lld)
		mas_sec_size = lld->mas_sec_size;

	if (!blk_queue_query_unistore_enable(q))
		return iter.bi_size;

	if (!mas_sec_size) {
		pr_err("%s - section size = %u\n", __func__, mas_sec_size);
		return iter.bi_size;
	}

	return ((mas_sec_size - ((iter.bi_sector >> SECTION_SECTOR) % mas_sec_size))
			<< SECTION_SECTOR) << SECTOR_BYTE;
}

bool mas_blk_bio_check_over_section(
	struct request_queue *q, struct bio *bio)
{
	unsigned residual_bytes;
	struct bvec_iter iter;

	if ((bio_op(bio) != REQ_OP_WRITE) ||
		!blk_queue_query_unistore_enable(q) ||
		!mas_blk_is_order_stream(bio->mas_bio.stream_type))
		return false;

	iter = bio->bi_iter;
	residual_bytes = mas_blk_bio_get_residual_byte(q, iter);
	if (iter.bi_size > residual_bytes)
		return true;

	return false;
}

void mas_blk_queue_unistore_enable(struct request_queue *q, bool enable)
{
	struct blk_dev_lld *lld = mas_blk_get_lld(q);
	if (enable)
		lld->features |= BLK_LLD_UFS_UNISTORE_EN;
	else
		lld->features &= ~BLK_LLD_UFS_UNISTORE_EN;
}

static void mas_blk_mq_free_map_and_requests(
	struct blk_mq_tag_set *set)
{
	unsigned int i;

	for (i = 0; i < set->nr_hw_queues; i++)
		blk_mq_free_map_and_requests(set, i);
}

int mas_blk_mq_update_unistore_tags(struct blk_mq_tag_set *set)
{
	int ret;

	if (!set)
		return -EINVAL;

	mas_blk_mq_free_map_and_requests(set);

	ret = blk_mq_alloc_rq_maps(set);
	if (ret)
		pr_err("%s alloc rq maps fail %d\n", __func__, ret);

	return ret;
}

unsigned int mas_blk_get_sec_size(struct request_queue *q)
{
	struct blk_dev_lld *lld = mas_blk_get_lld(q);

	return lld->mas_sec_size;
}

static void mas_blk_set_sec_size(struct request_queue *q, unsigned int mas_sec_size)
{
	struct blk_dev_lld *lld = mas_blk_get_lld(q);

	lld->mas_sec_size = mas_sec_size;
}

unsigned int mas_blk_get_pu_size(struct request_queue *q)
{
	struct blk_dev_lld *lld = mas_blk_get_lld(q);

	return lld->mas_pu_size;
}

static void mas_blk_set_pu_size(struct request_queue *q, unsigned int mas_pu_size)
{
	struct blk_dev_lld *lld = mas_blk_get_lld(q);

	lld->mas_pu_size = mas_pu_size;
}

void mas_blk_set_up_unistore_env(struct request_queue *q,
	unsigned int mas_sec_size, unsigned int mas_pu_size, bool enable)
{
	mas_blk_queue_unistore_enable(q, enable);

	if (enable) {
		mas_blk_set_pu_size(q, mas_pu_size);
		mas_blk_set_sec_size(q, mas_sec_size);
		mas_blk_bad_block_notify_work_init(q);
	}
}

bool blk_queue_query_unistore_enable(struct request_queue *q)
{
	struct blk_dev_lld *lld = NULL;

	if (!q)
		return false;

	lld = mas_blk_get_lld(q);

	return ((lld->features & BLK_LLD_UFS_UNISTORE_EN) ? true : false);
}

void mas_blk_set_recovery_flag(struct request_queue *q)
{
	struct blk_dev_lld *lld = mas_blk_get_lld(q);

	mutex_lock(&lld->recovery_mutex);
	if (!atomic_read(&lld->recovery_flag)) {
		down_write(&lld->recovery_rwsem);
		atomic_set(&lld->recovery_flag, 1);
	}

	if (!atomic_read(&lld->recovery_pwron_inprocess_flag))
		atomic_set(&lld->recovery_pwron_flag, 1);

	mutex_unlock(&lld->recovery_mutex);

	pr_err("%s recovery_pwron_flag %d recovery_pwron_inprocess_flag %d\n",
		__func__, atomic_read(&lld->recovery_pwron_flag),
		atomic_read(&lld->recovery_pwron_inprocess_flag));

	atomic_inc(&lld->reset_cnt);
}

static struct request *mas_blk_mq_alloc_request_reset(
	struct request_queue *q, unsigned int op, unsigned int flags)
{
	struct blk_mq_alloc_data alloc_data = { .flags = flags };
	struct request *rq = NULL;
	int ret;

	ret = blk_queue_enter(q, flags & BLK_MQ_REQ_NOWAIT);
	if (ret)
		return ERR_PTR(ret);

	alloc_data.mas_io_flag |= MAS_REQ_RESET;
	if (q->mas_queue_ops && q->mas_queue_ops->mq_req_alloc_prep_fn)
		q->mas_queue_ops->mq_req_alloc_prep_fn(&alloc_data, op, false);
	op |= (REQ_SYNC | REQ_FG);

	rq = blk_mq_get_request(q, NULL, op, &alloc_data);
	blk_queue_exit(q);

	if (!rq)
		return ERR_PTR(-EWOULDBLOCK);

	blk_mq_put_ctx(alloc_data.ctx);

	rq->__data_len = 0;
	rq->__sector = (sector_t) -1;
	rq->bio = rq->biotail = NULL;
	return rq;
}

struct request *mas_blk_get_request_reset(struct request_queue *q,
	unsigned int op, gfp_t gfp_mask)
{
	struct request *req = mas_blk_mq_alloc_request_reset(q, op,
		(gfp_mask & __GFP_DIRECT_RECLAIM) ?
			0 : BLK_MQ_REQ_NOWAIT);
	if (!IS_ERR(req) && q->mq_ops->initialize_rq_fn)
		q->mq_ops->initialize_rq_fn(req);

	return req;
}
