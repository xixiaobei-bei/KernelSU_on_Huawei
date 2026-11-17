/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
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
#include <linux/hisi/powerkey_event.h>
#include <trace/events/block.h>
#include <trace/iotrace.h>
#include <linux/types.h>
#include <scsi/scsi_host.h>
#include "blk.h"
#include "dsm_block.h"

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
		if (mas_blk_unistore_debug_en() && lld->mas_sec_size)
			pr_err("%s, new_stream_type = %u, old_stream_type = %u, "
				"fsync_ind = %d, cur_order = %u, pre_order_cnt = %u, "
				"lba = 0x%llx - 0x%llx\n",
				__func__, new_stream_type, lld->last_stream_type,
				req->mas_req.fsync_ind, *order, *pre_order_cnt,
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

static void mas_blk_del_section_list(struct blk_dev_lld *lld,
	sector_t section_start_lba, unsigned char stream_type)
{
	struct unistore_section_info *section_info = NULL;

	if ((!stream_type) || (stream_type > MAX_WRITE_STREAM_TYPE))
		return;

#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
	if (mas_blk_unistore_debug_en() && lld->mas_sec_size)
		pr_err("%s, section = 0x%llx, stream_type = %u\n", __func__,
			section_start_lba / (lld->mas_sec_size), stream_type);
#endif

	list_for_each_entry(section_info,
		&lld->section_list[stream_type - 1], section_list) {
		if (section_info->section_start_lba == section_start_lba)
			break;
	}
	if (&section_info->section_list != &lld->section_list[stream_type - 1]) {
		list_del_init(&section_info->section_list);
		kfree(section_info);
	}
}

static inline bool mas_blk_judge_consecutive_section(
	sector_t section_1, sector_t section_2)
{
	return ((section_1 == section_2) || (section_1 == section_2 + 1));
}

static inline void mas_blk_update_lba_time(struct blk_dev_lld *lld,
	sector_t update_lba, unsigned char stream_type)
{
	lld->expected_lba[stream_type - 1] = update_lba;
	lld->expected_refresh_time[stream_type - 1] = ktime_get();
}

static inline void mas_blk_update_section(struct blk_dev_lld *lld,
	sector_t expected_section, unsigned char stream_type)
{
	lld->old_section[stream_type - 1] = expected_section;
	mas_blk_del_section_list(lld, (lld->old_section[stream_type - 1] *
			(lld->mas_sec_size)), stream_type);
}

void mas_blk_update_expected_lba(struct request *req, unsigned int nr_bytes)
{
	struct blk_dev_lld *lld = mas_blk_get_lld(req->q);
	sector_t current_lba = blk_rq_pos(req) >> SECTION_SECTOR;
	sector_t update_lba;
	sector_t update_section;
	sector_t expected_section;
	bool consecutive_section = false;
	unsigned char stream_type = req->mas_req.stream_type;
	unsigned long flags = 0;

	if ((req_op(req) != REQ_OP_WRITE) || !(lld->features & BLK_LLD_UFS_UNISTORE_EN))
		return;

	if ((!stream_type) || (stream_type > MAX_WRITE_STREAM_TYPE))
		return;

	if (!lld->mas_sec_size)
		return;

	update_lba = current_lba + ((nr_bytes >> SECTOR_BYTE) >> SECTION_SECTOR);
	if (update_lba % (lld->mas_sec_size) == (lld->mas_sec_size / 3) && req->mas_req.slc_mode)
		update_lba += ((lld->mas_sec_size) - (lld->mas_sec_size / 3)); /* tlc : slc = 3 */
	update_section = update_lba / (lld->mas_sec_size);

	spin_lock_irqsave(&lld->expected_lba_lock[stream_type - 1], flags);
	expected_section = lld->expected_lba[stream_type - 1] / (lld->mas_sec_size);
	consecutive_section = mas_blk_judge_consecutive_section(update_section, expected_section);

	if (!lld->expected_lba[stream_type - 1]) {
		mas_blk_update_lba_time(lld, update_lba, stream_type);
	} else if (consecutive_section && (update_lba > lld->expected_lba[stream_type - 1])) {
		mas_blk_update_lba_time(lld, update_lba, stream_type);
		if (update_section == expected_section + 1)
			mas_blk_update_section(lld, expected_section, stream_type);
	} else if (!consecutive_section) {
		if (lld->old_section[stream_type - 1] &&
			mas_blk_judge_consecutive_section(update_section,
				lld->old_section[stream_type - 1])) {
			goto out;
		} else {
			if ((lld->expected_lba[stream_type - 1] % (lld->mas_sec_size)))
				mas_blk_update_section(lld, expected_section, stream_type);
			mas_blk_update_lba_time(lld, update_lba, stream_type);
		}
	}

out:
	spin_unlock_irqrestore(&lld->expected_lba_lock[stream_type - 1], flags);
	return;
}

bool mas_blk_match_expected_lba(struct request_queue *q, struct bio *bio)
{
	unsigned long flags = 0;
	struct blk_dev_lld *lld = NULL;
	sector_t current_lba;
	unsigned char stream_type;
	struct unistore_section_info *section_info = NULL;
	sector_t section_start_lba = 0;

	if (!q || !bio)
		return false;

	lld = mas_blk_get_lld(q);
	if (!(lld->features & BLK_LLD_UFS_UNISTORE_EN))
		return false;

	current_lba = bio->bi_iter.bi_sector >> SECTION_SECTOR;
	stream_type = bio->mas_bio.stream_type;

	if ((!stream_type) || (stream_type > MAX_WRITE_STREAM_TYPE))
		return false;

	spin_lock_irqsave(&lld->expected_lba_lock[stream_type - 1], flags);
	if (!list_empty_careful(&lld->section_list[stream_type - 1])) {
		section_info = list_first_entry(
				&lld->section_list[stream_type - 1],
				struct unistore_section_info, section_list);
		section_start_lba = section_info->section_start_lba;
	}
	if ((lld->expected_lba[stream_type - 1] == current_lba) ||
					(current_lba == section_start_lba)) {
		spin_unlock_irqrestore(&lld->expected_lba_lock[stream_type - 1], flags);
		return true;
	}
	spin_unlock_irqrestore(&lld->expected_lba_lock[stream_type - 1], flags);
	return false;
}

static void mas_blk_put_page_in_buf_list(struct bio* bio)
{
	int i;
	struct page *ori_page = NULL;
	struct bio_vec *bvec = NULL;

	bio_for_each_segment_all(bvec, bio, i) {
		ori_page = bvec->bv_page;
		if (!PageAnon(ori_page))
			ClearPageCached(ori_page);

		put_page(ori_page);
	}
}

static void mas_blk_get_page_in_buf_list(struct bio* bio)
{
	int i;
	struct page *ori_page = NULL;
	struct bio_vec *bvec = NULL;

	bio_for_each_segment_all(bvec, bio, i) {
		ori_page = bvec->bv_page;
		get_page(ori_page);

		if (!PageAnon(ori_page))
			SetPageCached(ori_page);
	}
}

static bool mas_blk_update_bio_page(struct bio *pos,
	struct page *page, struct page *cached_page)
{
	int j;
	struct bio_vec *bvec = NULL;

	bio_for_each_segment_all(bvec, pos, j) {
		if (bvec->bv_page == page) {
			bvec->bv_page = cached_page;
			SetPageCached(cached_page);
			ClearPageCached(page);
			put_page(page);
			return true;
		}
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
	for (i = 0; i < MAX_WRITE_STREAM_TYPE + 1; i++) {
		spin_lock_irqsave(&lld->buf_bio_list_lock[i], flag);
		if (list_empty_careful(&lld->buf_bio_list[i])) {
			spin_unlock_irqrestore(&lld->buf_bio_list_lock[i], flag);
			continue;
		}

		list_for_each_entry(pos, &lld->buf_bio_list[i], buf_bio_list_node) {
			if (mas_blk_update_bio_page(pos, page, cached_page)) {
#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
				if (mas_blk_unistore_debug_en() && lld->mas_sec_size)
					pr_err("%s, page:%pK, bio:%pK, "
						"lba:0x%llx - 0x%llx is replaced\n",
						__func__, page, pos,
						(pos->bi_iter.bi_sector >> SECTION_SECTOR) /
							(lld->mas_sec_size),
						(pos->bi_iter.bi_sector >> SECTION_SECTOR) %
							(lld->mas_sec_size));
#endif
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

#define BLKSIZE 4096 /* bytes */
#define BIO_BUF_LIST_MAX_SIZE (2 * 1024 * 1024) /* buffer bio max bytes */
#define MAX_CLEAR_BUF_BIO_PER_STREAM 500 /* max bio number per stream */
static void mas_blk_del_bio_in_buf_list(struct blk_dev_lld *lld,
	struct bio *pos, unsigned char stream_type)
{
	list_del_init(&pos->buf_bio_list_node);
	lld->buf_bio_size[stream_type] -= pos->ori_bi_iter.bi_size;
	lld->buf_bio_num[stream_type]--;
	pos->buf_bio = false;
	mas_blk_put_page_in_buf_list(pos);
	bio_put(pos);
}

static inline bool mas_blk_bio_nr_before(unsigned int nr1, unsigned int nr2)
{
	return (signed int)(nr1 - nr2) > 0;
}

static void mas_blk_order_insert_buf_list(struct bio *bio,
	struct blk_dev_lld *lld, unsigned char stream_type)
{
	struct list_head *target_list = NULL;
	struct bio *pos = NULL;

	target_list = &lld->buf_bio_list[stream_type];

#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
	if (mas_blk_unistore_debug_en() && lld->mas_sec_size)
		pr_err("%s - bio_nr=%u - bio_lba=0x%llx - 0x%llx\n",
			__func__, bio->bio_nr,
			(bio->bi_iter.bi_sector >> SECTION_SECTOR) /
				(lld->mas_sec_size),
			(bio->bi_iter.bi_sector >> SECTION_SECTOR) %
				(lld->mas_sec_size));
#endif
	if (!list_empty(target_list) && bio->bio_nr) {
		list_for_each_entry_reverse(pos, target_list, buf_bio_list_node) {
			if (!pos->bio_nr)
				continue;
			if (mas_blk_bio_nr_before(bio->bio_nr, pos->bio_nr)) {
				list_add_tail(&bio->buf_bio_list_node,
					&pos->buf_bio_list_node);
				break;
			}
		}
		if (&pos->buf_bio_list_node == target_list)
			list_add_tail(&bio->buf_bio_list_node,
				&pos->buf_bio_list_node);
	} else {
		list_add_tail(&bio->buf_bio_list_node, target_list);
	}
}

static void mas_blk_buf_list_update(struct blk_dev_lld *lld,
	struct bio *bio, unsigned char stream_type)
{
	struct bio *pos = NULL;

	mas_blk_order_insert_buf_list(bio, lld, stream_type);
	bio_get(bio);
	bio->buf_bio = true;
	memcpy(&(bio->ori_bi_iter), &(bio->bi_iter), sizeof(struct bvec_iter));
	mas_blk_get_page_in_buf_list(bio);
	lld->buf_bio_size[stream_type] += bio->ori_bi_iter.bi_size;
	lld->buf_bio_num[stream_type]++;

#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
	if (mas_blk_unistore_debug_en() && lld->mas_sec_size)
		pr_err("%s - buf_io_num = %u - add_bio_lba: 0x%llx - 0x%llx\n",
			__func__, lld->buf_bio_num[stream_type],
			(bio->bi_iter.bi_sector >> SECTION_SECTOR) /
				(lld->mas_sec_size),
			(bio->bi_iter.bi_sector >> SECTION_SECTOR) %
				(lld->mas_sec_size));
#endif
	if (lld->buf_bio_size[stream_type] > BIO_BUF_LIST_MAX_SIZE) {
		pos = list_first_entry(&lld->buf_bio_list[stream_type],
					struct bio, buf_bio_list_node);
#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
		if (mas_blk_unistore_debug_en() && lld->mas_sec_size)
			pr_err("%s - buf_io_num = %u, del_bio_lba: 0x%llx - 0x%llx\n",
				__func__, lld->buf_bio_num[stream_type],
				(pos->bi_iter.bi_sector >> SECTION_SECTOR) /
					(lld->mas_sec_size),
				(pos->bi_iter.bi_sector >> SECTION_SECTOR) %
					(lld->mas_sec_size));
#endif
		mas_blk_del_bio_in_buf_list(lld, pos, stream_type);
	}
}

void mas_blk_add_bio_to_buf_list(struct request *req)
{
	int i;
	unsigned long flags;
	unsigned char stream_type;
	struct blk_dev_lld *lld = NULL;
	struct bio *bio = NULL;

	if (!blk_queue_query_unistore_enable(req->q))
		return;

	if (!(req->mas_req.stream_type) ||
		(req->mas_req.stream_type == STREAM_TYPE_INVALID))
		return;

	if (!(req->bio) || !(op_is_write(bio_op(req->bio))))
		return;

	if (req->bio->buf_bio)
		return;

	bio = req->bio;
	bio->bio_nr = req->mas_req.make_req_nr;
	stream_type = req->bio->mas_bio.stream_type;
	lld = mas_blk_get_lld(req->q);
	if (!lld)
		return;

	spin_lock_irqsave(&lld->buf_bio_list_lock[stream_type], flags);
	mas_blk_buf_list_update(lld, bio, stream_type);
	spin_unlock_irqrestore(&lld->buf_bio_list_lock[stream_type], flags);

	if (lld->buf_bio_size[stream_type] > BIO_BUF_LIST_MAX_SIZE) {
		for (i = 1; i <= NR_CPUS; i++) {
			if (cpu_online((raw_smp_processor_id() + i) % NR_CPUS))
				break;
		}
		schedule_delayed_work_on(
			(raw_smp_processor_id() + i) % NR_CPUS,
			&lld->clear_buf_bio_work, 0);
	}
}

static void mas_blk_clear_buf_bio_work(struct work_struct *work)
{
	struct blk_dev_lld *lld = NULL;
	struct bio *pos = NULL;
	unsigned long flags;
	unsigned char stream;
	int j;

	lld = container_of(work, struct blk_dev_lld, clear_buf_bio_work.work);

	for (stream = 0; stream < MAX_WRITE_STREAM_TYPE + 1; stream++) {
		j = 0;
		while ((lld->buf_bio_size[stream] > BIO_BUF_LIST_MAX_SIZE) &&
			(j < MAX_CLEAR_BUF_BIO_PER_STREAM)) {
			spin_lock_irqsave(&lld->buf_bio_list_lock[stream], flags);
			if (list_empty_careful(&lld->buf_bio_list[stream])) {
				spin_unlock_irqrestore(&lld->buf_bio_list_lock[stream], flags);
				break;
			}

			pos = list_first_entry(
				&lld->buf_bio_list[stream],
				struct bio, buf_bio_list_node);
#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
			if (mas_blk_unistore_debug_en() && lld->mas_sec_size)
				pr_err("%s, stream = %u, buf_num = %u, buf_size = %u, "
					"del_bio_lba: 0x%llx - 0x%llx\n",
					__func__, stream,
					lld->buf_bio_num[stream],
					lld->buf_bio_size[stream],
					(pos->bi_iter.bi_sector >> SECTION_SECTOR) /
						(lld->mas_sec_size),
					(pos->bi_iter.bi_sector >> SECTION_SECTOR) %
						(lld->mas_sec_size));
#endif
			mas_blk_del_bio_in_buf_list(lld, pos, stream);
			j++;
			spin_unlock_irqrestore(&lld->buf_bio_list_lock[stream], flags);
		}
	}
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

	if (bio->mas_bio.stream_type > MAX_WRITE_STREAM_TYPE)
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

#if defined(CONFIG_MAS_DEBUG_FS) || defined(CONFIG_MAS_BLK_DEBUG)
ssize_t mas_queue_unistore_en_show(struct request_queue *q, char *page)
{
	unsigned long offset = 0;
	struct blk_dev_lld *lld = mas_blk_get_lld(q);

	offset += snprintf(page, PAGE_SIZE, "unistore_enabled: %d\n",
		(lld->features & BLK_LLD_UFS_UNISTORE_EN) ? 1 : 0);

	return (ssize_t)offset;
}

static int unistore_debug_en;
int mas_blk_unistore_debug_en(void)
{
	return unistore_debug_en;
}

ssize_t mas_queue_unistore_debug_en_show(struct request_queue *q, char *page)
{
	unsigned long offset;
	offset = snprintf(page, PAGE_SIZE, "unistore_debug_en: %d\n", unistore_debug_en);
	return (ssize_t)offset;
}

ssize_t mas_queue_unistore_debug_en_store(
	struct request_queue *q, const char *page, size_t count)
{
	ssize_t ret;
	unsigned long val;

	ret = queue_var_store(&val, page, count);
	if (ret < 0)
		return (ssize_t)count;

	if (val)
		unistore_debug_en = val;
	else
		unistore_debug_en = 0;

	return (ssize_t)count;
}
#endif

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

void mas_blk_dev_lld_init_unistore(struct blk_dev_lld *blk_lld)
{
	unsigned int i;

	blk_lld->unistore_ops.dev_bad_block_notfiy_fn = NULL;
	blk_lld->unistore_ops.dev_bad_block_notfiy_param_data = NULL;
	blk_lld->last_stream_type = STREAM_TYPE_INVALID;
	blk_lld->fsync_ind = false;
	blk_lld->mas_sec_size = 0;
	spin_lock_init(&blk_lld->fsync_ind_lock);
	for (i = 0; i < MAX_WRITE_STREAM_TYPE; i++) {
		spin_lock_init(&blk_lld->expected_lba_lock[i]);
		blk_lld->expected_lba[i] = 0;
		blk_lld->expected_refresh_time[i] = 0;
		blk_lld->old_section[i] = 0;
		INIT_LIST_HEAD(&blk_lld->section_list[i]);
	}
	for (i = 0; i < MAX_WRITE_STREAM_TYPE + 1; i++) {
		INIT_LIST_HEAD(&blk_lld->buf_bio_list[i]);
		spin_lock_init(&blk_lld->buf_bio_list_lock[i]);
		blk_lld->buf_bio_size[i] = 0;
		blk_lld->buf_bio_num[i] = 0;
	}
	atomic_set(&blk_lld->recovery_flag, 0);
	init_rwsem(&blk_lld->recovery_rwsem);
	mutex_init(&blk_lld->recovery_mutex);
	INIT_DELAYED_WORK(&blk_lld->clear_buf_bio_work, mas_blk_clear_buf_bio_work);

	blk_lld->write_curr_cnt = 0;
	blk_lld->write_pre_cnt = 0;

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
		if (bio->mas_bio.stream_type > 0)
			bio->bi_opf &= ~REQ_PREFLUSH;
		bio->bi_opf |= REQ_SYNC;
	}
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

void mas_blk_set_up_unistore_env(struct request_queue *q,
	unsigned int mas_sec_size, bool enable)
{
	mas_blk_queue_unistore_enable(q, enable);

	if (enable) {
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
