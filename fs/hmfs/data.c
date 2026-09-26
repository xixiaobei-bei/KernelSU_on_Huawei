// SPDX-License-Identifier: GPL-2.0
/*
 * fs/f2fs/data.c
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 */
#include <linux/fs.h>
#include <linux/hmfs_fs.h>
#include <linux/buffer_head.h>
#include <linux/mpage.h>
#include <linux/writeback.h>
#include <linux/backing-dev.h>
#include <linux/pagevec.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/prefetch.h>
#include <linux/uio.h>
#include <linux/cleancache.h>
#include <linux/sched/signal.h>
#include <linux/hrtimer.h>

#include "hmfs.h"
#include "node.h"
#include "segment.h"
#include "trace.h"
#include "gc.h"
#include <trace/events/hmfs.h>
#include <trace/events/android_fs.h>

#define NUM_PREALLOC_POST_READ_CTXS	128

#define US_TO_NS		1000

static struct kmem_cache *bio_post_read_ctx_cache;
static mempool_t *bio_post_read_ctx_pool;

static bool __is_pu_align_enabled(struct f2fs_sb_info *sbi, int type)
{
	/* from UFS limits */
	return (type == CURSEG_COLD_DATA || type == CURSEG_COLD_NODE);
}

static bool __is_cp_guaranteed(struct page *page)
{
	struct address_space *mapping = page->mapping;
	struct inode *inode;
	struct f2fs_sb_info *sbi;

	if (!mapping)
		return false;

	inode = mapping->host;
	sbi = F2FS_I_SB(inode);

	if (inode->i_ino == F2FS_META_INO(sbi) ||
			inode->i_ino ==  F2FS_NODE_INO(sbi) ||
			S_ISDIR(inode->i_mode) ||
			(S_ISREG(inode->i_mode) &&
			(f2fs_is_atomic_file(inode) || IS_NOQUOTA(inode))) ||
			is_cold_data(page))
		return true;
	return false;
}

static void set_page_wb_type(struct page *page, bool is_cp_data)
{
	if (is_cp_data)
		SetPageCPdata(page);
	else
		ClearPageCPdata(page);
}

static enum count_type get_page_wb_type(struct page *page)
{
	return PageCPdata(page) ? F2FS_WB_CP_DATA : F2FS_WB_DATA;
}

static enum count_type __read_io_type(struct page *page)
{
	struct address_space *mapping = page->mapping;

	if (mapping) {
		struct inode *inode = mapping->host;
		struct f2fs_sb_info *sbi = F2FS_I_SB(inode);

		if (inode->i_ino == F2FS_META_INO(sbi))
			return F2FS_RD_META;

		if (inode->i_ino == F2FS_NODE_INO(sbi))
			return F2FS_RD_NODE;
	}
	return F2FS_RD_DATA;
}

/* postprocessing steps for read bios */
enum bio_post_read_step {
	STEP_INITIAL = 0,
	STEP_DECRYPT,
};

struct bio_post_read_ctx {
	struct bio *bio;
	struct work_struct work;
	unsigned int cur_step;
	unsigned int enabled_steps;
};

static void hmfs_gc_wait(struct f2fs_sb_info *sbi)
{
	DEFINE_WAIT(wait);

	if (atomic_read(&(sbi->gc_control_info.inflight)) <
			sbi->gc_control_info.iolimit)
		return;

	do {
		prepare_to_wait_exclusive(&(sbi->gc_control_info.wait), &wait,
						TASK_UNINTERRUPTIBLE);

		if (atomic_read(&(sbi->gc_control_info.inflight)) <
					sbi->gc_control_info.iolimit)
			break;

		io_schedule();
	} while (1);

	finish_wait(&(sbi->gc_control_info.wait), &wait);
}

static void __read_end_io(struct bio *bio)
{
	struct page *page;
	struct bio_vec *bv;
	int i;

	bio_for_each_segment_all(bv, bio, i) {
		page = bv->bv_page;

		/* PG_error was set if any post_read step failed */
		if (bio->bi_status || PageError(page)) {
			 printk(KERN_ERR "%s: read IO fault[%d]\n",
				 __func__, bio->bi_status);
#ifdef CONFIG_HUAWEI_HMFS_DSM
			/* report this behavor to DSM */
			if (hmfs_dclient && !dsm_client_ocuppy(hmfs_dclient)) {
				dsm_client_record(hmfs_dclient,
				"%s: read IO fault[%d]\n",
				__func__, bio->bi_status);
				dsm_client_notify(hmfs_dclient, DSM_HMFS_NEED_FSCK);
			}
#endif
			ClearPageUptodate(page);
			/* will re-read again later */
			ClearPageError(page);
		} else {
			SetPageUptodate(page);
		}
		dec_page_count(F2FS_P_SB(page), __read_io_type(page));
		unlock_page(page);
	}
	if (bio->bi_private)
		mempool_free(bio->bi_private, bio_post_read_ctx_pool);
	bio_put(bio);
}

static void bio_post_read_processing(struct bio_post_read_ctx *ctx);

static void hmfs_fio_check(struct f2fs_io_info *fio)
{
	if (!is_read_io(fio->op) &&
	    fio->new_blkaddr == fio->old_blkaddr &&
	    (fio->type == DATA || fio->type == NODE))
		f2fs_bug_on(fio->sbi, 1);
}

static void decrypt_work(struct work_struct *work)
{
	struct bio_post_read_ctx *ctx =
		container_of(work, struct bio_post_read_ctx, work);

	fscrypt_decrypt_bio(ctx->bio);

	bio_post_read_processing(ctx);
}

static void bio_post_read_processing(struct bio_post_read_ctx *ctx)
{
	switch (++ctx->cur_step) {
	case STEP_DECRYPT:
		if (ctx->enabled_steps & (1 << STEP_DECRYPT)) {
			INIT_WORK(&ctx->work, decrypt_work);
			fscrypt_enqueue_decrypt_work(&ctx->work);
			return;
		}
		ctx->cur_step++;
		/* fall-through */
	default:
		__read_end_io(ctx->bio);
	}
}

static bool f2fs_bio_post_read_required(struct bio *bio)
{
	return bio->bi_private && !bio->bi_status;
}

static void hmfs_dm_endio(struct bio *bio)
{
	struct hmfs_bio *hio = HMFS_BIO(bio);
	struct hmfs_dm_manager *dm = HMFS_DM(hio->sbi);

	if (likely(!(hio->flags & HMFS_BIO_DM_REF)))
		return;

	if (likely(!atomic_sub_and_test(hio->mapped_blks, &dm->refs)))
		return;

	wake_up(&dm->wait);
}

static void f2fs_read_end_io(struct bio *bio)
{
	hmfs_dm_endio(bio);

	if (time_to_inject(F2FS_P_SB(bio->bi_io_vec->bv_page), FAULT_READ_IO)) {
		f2fs_show_injection_info(FAULT_READ_IO);
		bio->bi_status = BLK_STS_IOERR;
	}

	if (f2fs_bio_post_read_required(bio)) {
		struct bio_post_read_ctx *ctx = bio->bi_private;

		ctx->cur_step = STEP_INITIAL;
		bio_post_read_processing(ctx);
		return;
	}

	__read_end_io(bio);
}

static void f2fs_write_end_io(struct bio *bio)
{
	struct f2fs_sb_info *sbi = bio->bi_private;
#ifdef CONFIG_HMFS_CHECK_FS
	struct seg_entry *se = NULL;
	unsigned int segno;
	block_t blkaddr = bio->bi_start_blkaddr;
	nid_t nid;
#endif
	struct bio_vec *bvec;
	int i;

	hmfs_dm_endio(bio);

	if (time_to_inject(sbi, FAULT_WRITE_IO)) {
		f2fs_show_injection_info(FAULT_WRITE_IO);
		bio->bi_status = BLK_STS_IOERR;
	}

	bio_for_each_segment_all(bvec, bio, i) {
		struct page *page = bvec->bv_page;
		enum count_type type = get_page_wb_type(page);

		if (IS_DUMMY_WRITTEN_PAGE(page)) {
			/* dummy page feature is never used */
			f2fs_bug_on(sbi, 1);

			set_page_private(page, (unsigned long)NULL);/*lint !e527*/
			ClearPagePrivate(page);
			unlock_page(page);
			mempool_free(page, sbi->write_io_dummy);

			if (unlikely(bio->bi_status))
				hmfs_stop_checkpoint(sbi, true);
			continue;
		}

		fscrypt_pullback_bio_page(&page, true);

		if (unlikely(bio->bi_status)) {
			mapping_set_error(page->mapping, -EIO);
			if (type == F2FS_WB_CP_DATA)
				hmfs_stop_checkpoint(sbi, true);
			hmfs_msg(sbi->sb, KERN_ERR,
				"hmfs reboot for bio submit error! erro_num = %d\n", bio->bi_status);
#ifdef CONFIG_HUAWEI_HMFS_DSM
			if (hmfs_dclient && !dsm_client_ocuppy(hmfs_dclient)) {
				dsm_client_record(hmfs_dclient, "HMFS reboot: %s:%d [%d]\n",
					__func__, __LINE__, bio->bi_status);
				dsm_client_notify(hmfs_dclient, DSM_HMFS_NEED_FSCK);
			}
#endif
			f2fs_restart(); /* force restarting */
		}

#ifndef CONFIG_HMFS_CHECK_FS
		if (page->mapping == NODE_MAPPING(sbi) &&
					page->index != nid_of_node(page))
			f2fs_bug_on(sbi, 1);
#else
		if (page->mapping != META_MAPPING(sbi)) {
			segno = GET_SEGNO(sbi, blkaddr + i);
			if (segno != NULL_SEGNO)
				se = get_seg_entry(sbi, segno);
		}
		if (page->mapping == NODE_MAPPING(sbi)) {
			nid = nid_of_node(page);
			if (page->index != nid)
				f2fs_bug_on(sbi, 1);
			if (!se || !IS_NODESEG(se->type)) {
				pr_err("HMFS-fs: %s: invalid nid %u seg type %d "
					"expect NODE blk %#x\n", __func__, nid,
					se ? se->type : -1, blkaddr + i);
				f2fs_bug_on(sbi, 1);
			}
		} else if (page->mapping != META_MAPPING(sbi)) {
			/* should be data block */
			if (!se || !IS_DATASEG(se->type)) {
				pr_err("HMFS-fs: %s: invalid seg type %d expect "
					"DATA blk %#x\n", __func__, se ? se->type : -1,
					blkaddr + i);
				f2fs_bug_on(sbi, 1);
			}
		}
#endif
		dec_page_count(sbi, type);
		ClearPageCPdata(page);
		if (hmfs_in_warm_node_list(sbi, page))
			hmfs_del_fsync_node_entry(sbi, page);
		clear_cold_data(page);
		end_page_writeback(page);
	}
	if (!get_pages(sbi, F2FS_WB_CP_DATA) &&
				wq_has_sleeper(&sbi->cp_wait))
		wake_up(&sbi->cp_wait);

#ifdef CONFIG_MAS_BLK
	if (bio->mas_bi_opf & MAS_REQ_GC)
		atomic_dec(&(sbi->gc_control_info.inflight));

	/*
	 * gc io_limit workqueue need to be awake if
	 * inflight io less than threshold
	 */
	if (atomic_read(&(sbi->gc_control_info.inflight)) <
			sbi->gc_control_info.iolimit &&
			waitqueue_active(&(sbi->gc_control_info.wait)))
		wake_up(&(sbi->gc_control_info.wait));
#endif

	bio_put(bio);
}

/*
 * Return true, if pre_bio's bdev is same as its target device.
 */
struct block_device *hmfs_target_device(struct f2fs_sb_info *sbi,
				block_t blk_addr, struct bio *bio)
{
	struct block_device *bdev = sbi->sb->s_bdev;
	int i;

#ifdef CONFIG_HMFS_CHECK_FS
	if (bio)
		bio->bi_start_blkaddr = blk_addr;
#endif

	for (i = 0; i < sbi->s_ndevs; i++) {
		if (FDEV(i).start_blk <= blk_addr &&
					FDEV(i).end_blk >= blk_addr) {
			blk_addr -= FDEV(i).start_blk;
			bdev = FDEV(i).bdev;
			break;
		}
	}
	if (bio) {
		bio_set_dev(bio, bdev);
		bio->bi_iter.bi_sector = SECTOR_FROM_BLOCK(blk_addr);
	}
	return bdev;
}

int hmfs_target_device_index(struct f2fs_sb_info *sbi, block_t blkaddr)
{
	int i;

	for (i = 0; i < sbi->s_ndevs; i++)
		if (FDEV(i).start_blk <= blkaddr && FDEV(i).end_blk >= blkaddr)
			return i;
	return 0;
}

/*
 * Low-level block read/write IO operations.
 */
static struct bio *__bio_alloc(struct f2fs_sb_info *sbi, block_t blk_addr,
				struct writeback_control *wbc,
				int npages, bool is_read,
				enum page_type type, enum temp_type temp)
{
	struct bio *bio;

	bio = f2fs_bio_alloc(sbi, npages, true);

	hmfs_target_device(sbi, blk_addr, bio);
	if (is_read) {
		bio->bi_end_io = f2fs_read_end_io;
		bio->bi_private = NULL;
	} else {
		bio->bi_end_io = f2fs_write_end_io;
		bio->bi_private = sbi;
		bio->bi_write_hint = hmfs_io_type_to_rw_hint(sbi, type, temp);
	}
	if (wbc)
		wbc_init_bio(wbc, bio);

#ifdef CONFIG_HMFS_FS_ENCRYPTION
	bio->ci_key = NULL;
	bio->ci_key_len = 0;
	bio->ci_key_index = -1;
	bio->ci_metadata = NULL;
#endif
	return bio;
}

static inline void hmfs_bio_set_stream(struct f2fs_sb_info *sbi, struct bio *bio,
		enum page_type type, int temp)
{
	int stream_id;
	if (is_read_io(bio_op(bio)))
		return;

	stream_id = get_stream_id_by_type_temp(type, temp);
	if (stream_id != STREAM_NR) {
		bio->mas_bio.stream_type = stream_id;
	} else {
		hmfs_msg(sbi->sb, KERN_ERR, "%s: type%d, temp%u\n",
			__func__, type, temp);
		WARN_ON(1);
	}
}

static void bio_set_oob(struct f2fs_sb_info *sbi,
                        struct bio *bio, enum page_type type,
                        unsigned long ino, loff_t fofs)
{
	struct f2fs_inode_info *fi;

	if (!hmfs_is_oob_enable(sbi))
		return;
	if (is_read_io(bio_op(bio)))
		return;

	/* only for DATA: NODE has ino/nid in node_footer */
	if (type == DATA) {
		struct page *page = bio->bi_io_vec->bv_page;

		if (!page->mapping) {
			page = fscrypt_control_page(bio->bi_io_vec->bv_page);
		}
		fi = F2FS_I(page->mapping->host);
		f2fs_bug_on(sbi, fofs & OOB_FSYNC_BIT);

		bio->mas_bio.data_ino = ino;
		if ((fi->fsync_task == current) &&
			(!fi->fsync_atomic ||
			(fi->oob_last_page &&
			fi->oob_last_page->index == page->index)))
			fofs |= OOB_FSYNC_BIT;

		bio->mas_bio.data_idx = fofs;

		hmfs_file_check_switch_stream(page->mapping->host,
				bio->mas_bio.stream_type);
	}
}

static bool is_first_bio_in_section_list(struct f2fs_sb_info *sbi,
					block_t blkaddr, int stream_id)
{
	if ((atomic_read(&sbi->last_blkaddr[stream_id]) == NULL_ADDR) &&
			IS_FIRST_DATA_BLOCK_IN_SEC(sbi, blkaddr) &&
			(GET_SEC_FROM_SEG(sbi, GET_SEGNO(sbi, blkaddr)) ==
			 hmfs_section_order_list_peek(sbi, stream_id)))
		return true;
	else
		return false;
}

static bool is_legal_bio_for_issue(struct f2fs_sb_info *sbi,
					block_t blkaddr, int stream_id)
{
	if (atomic_read(&sbi->last_blkaddr[stream_id]) == blkaddr ||
			is_first_bio_in_section_list(sbi, blkaddr, stream_id)) {
		hmfs_section_order_list_pop(sbi, blkaddr, stream_id);
		return true;
	} else {
		return false;
	}
}

static inline struct bio* hmfs_find_and_pop_bio_in_list(struct f2fs_sb_info *sbi,
					struct bio_list *bl, int stream_id)
{
	struct bio *bio_pre = NULL;
	struct bio *tmp_bio = NULL;
	block_t tmp_blkaddr;

	bio_list_for_each(tmp_bio, bl) {
		tmp_blkaddr = hmfs_sector_to_blkaddr(sbi, tmp_bio);
		if (is_legal_bio_for_issue(sbi, tmp_blkaddr, stream_id)) {
			/* cannot enter this switch if last_blkaddr == bio_list_top_addr */
			f2fs_bug_on(sbi, bio_pre == NULL);

			bio_pre->bi_next = tmp_bio->bi_next;
			tmp_bio->bi_next = NULL;
			if (bl->tail == tmp_bio)
				bl->tail = bio_pre;
			break;
		}
		bio_pre = tmp_bio;
	}

	return tmp_bio;
}

static inline void __hmfs_submit_write_bio(struct f2fs_sb_info *sbi, struct bio *bio,
				enum page_type type)
{
	block_t start_block = hmfs_sector_to_blkaddr(sbi, bio);
	block_t tmp_blkaddr, end_blkaddr;
	int tmp_nr_blk;
	int stream_id = bio->mas_bio.stream_type;
	struct bio *bio_pre;
	struct bio *tmp_bio;
	struct bio_list *bl;

	f2fs_bug_on(sbi, (stream_id < STREAM_META) ||
				(stream_id >= STREAM_NR));
	f2fs_bug_on(sbi, ((stream_id == STREAM_META) &&
			     (start_block >=  MAIN_BLKADDR(sbi))));
	f2fs_bug_on(sbi, ((stream_id != STREAM_META) &&
			     (start_block <  MAIN_BLKADDR(sbi))));

	sbi->order_stat_times[0]++;
	if (stream_id == STREAM_META) {
		submit_bio(bio);
		return;
	}

	bl = &sbi->bio_list[stream_id];
	trace_hmfs_bio_list_pre_submit(bio,
			atomic_read(&sbi->last_blkaddr[stream_id]), type);

	tmp_nr_blk = SECTOR_TO_BLOCK(bio_sectors(bio));

	stat_inc_pu_info(sbi, PU_ALIGN_IO, tmp_nr_blk);
	if (__is_pu_align_enabled(sbi, CURSEG_T(stream_id))) {
		block_t pu_align_begin = start_block - MAIN_BLKADDR(sbi);
		block_t pu_align_len = tmp_nr_blk;

		if (pu_align_begin % sbi->pu_size[SLC_MODE])
			stat_inc_pu_info(sbi, PU_ALIGN_HEAD, tmp_nr_blk);
		else if (pu_align_len % sbi->pu_size[SLC_MODE])
			stat_inc_pu_info(sbi, PU_ALIGN_LEN, tmp_nr_blk);
	}

	mutex_lock(&sbi->bio_lock[stream_id]);
	if (is_legal_bio_for_issue(sbi, start_block, stream_id)) {
		trace_hmfs_bio_list_submit_direct(bio,
				atomic_read(&sbi->last_blkaddr[stream_id]), type);
		if (IS_LAST_DATA_BLOCK_IN_SEC(sbi, start_block + tmp_nr_blk - 1, DATA))
			atomic_set(&sbi->last_blkaddr[stream_id], NULL_ADDR);
		else
			atomic_set(&sbi->last_blkaddr[stream_id], start_block + tmp_nr_blk);
		mutex_unlock(&sbi->bio_lock[stream_id]);
		submit_bio(bio);
	} else {
		trace_hmfs_bio_list_insert(bio,
			atomic_read(&sbi->last_blkaddr[stream_id]), type);
		/* insert bio to f2fs_bio_list by stream id */
		if (bio_list_empty(bl)) {
			bio_list_add(bl, bio);
		} else {
			bio_pre = NULL;
			bio_list_for_each(tmp_bio, bl) {
				tmp_blkaddr = hmfs_sector_to_blkaddr(sbi, tmp_bio);
				if (start_block < tmp_blkaddr)
					break;
				bio_pre = tmp_bio;
			}
			if (bio_pre == NULL)
				bio_list_add_head(bl, bio);
			else
				bio_list_insert(bl, bio_pre, bio);
		}
		mutex_unlock(&sbi->bio_lock[stream_id]);
	}
	/* submit bio orderly */
	while (!bio_list_empty(bl)) {
		mutex_lock(&sbi->bio_lock[stream_id]);
		bl = &sbi->bio_list[stream_id];

		if (bio_list_empty(bl)) {
			mutex_unlock(&sbi->bio_lock[stream_id]);
			break;
		}
		tmp_blkaddr = hmfs_sector_to_blkaddr(sbi, bio_list_peek(bl));
		end_blkaddr = hmfs_sector_to_blkaddr(sbi, bl->tail);
		if (is_legal_bio_for_issue(sbi, tmp_blkaddr, stream_id)) {
			tmp_nr_blk = SECTOR_TO_BLOCK(bio_sectors(bio_list_peek(bl)));
			tmp_blkaddr += tmp_nr_blk;
			tmp_bio = bio_list_pop(bl);
			if (IS_LAST_DATA_BLOCK_IN_SEC(sbi, tmp_blkaddr - 1, DATA))
				atomic_set(&sbi->last_blkaddr[stream_id], NULL_ADDR);
			else
				atomic_set(&sbi->last_blkaddr[stream_id], tmp_blkaddr);
			mutex_unlock(&sbi->bio_lock[stream_id]);

			f2fs_bug_on(sbi, !tmp_bio);
			trace_hmfs_bio_list_submit_inlist(tmp_bio,
				atomic_read(&sbi->last_blkaddr[stream_id]), type);
			submit_bio(tmp_bio);
		} else if (!DATA_BLOCK_IN_SAME_SEC(sbi, tmp_blkaddr, end_blkaddr)) {
			sbi->order_stat_times[1]++;
			tmp_bio = hmfs_find_and_pop_bio_in_list(sbi, bl, stream_id);
			if (tmp_bio) {
				tmp_blkaddr = hmfs_sector_to_blkaddr(sbi, tmp_bio);
				tmp_nr_blk = SECTOR_TO_BLOCK(bio_sectors(tmp_bio));
				if (IS_LAST_DATA_BLOCK_IN_SEC(sbi, tmp_blkaddr + tmp_nr_blk - 1, DATA))
					atomic_set(&sbi->last_blkaddr[stream_id], NULL_ADDR);
				else
					atomic_set(&sbi->last_blkaddr[stream_id], tmp_blkaddr + tmp_nr_blk);
				mutex_unlock(&sbi->bio_lock[stream_id]);
				submit_bio(tmp_bio);
			} else {
				mutex_unlock(&sbi->bio_lock[stream_id]);
				break;
			}
		} else {
			mutex_unlock(&sbi->bio_lock[stream_id]);
			break;
		}
	}
}

static inline void __submit_bio(struct f2fs_sb_info *sbi,
				struct bio *bio, enum page_type type)
{
#ifdef CONFIG_BLK_DEV_THROTTLING
	if (!(current->flags & PF_MUTEX_GC))
		current->wb_stat.bios++;
#endif

	if (!is_read_io(bio_op(bio))) {
		unsigned int start;
#ifdef CONFIG_HMFS_CHECK_FS
		struct seg_entry *se;
		block_t blkaddr;
#endif

		if (type != DATA && type != NODE)
			goto submit_io;

		if (test_opt(sbi, LFS) && current->plug)
			blk_finish_plug(current->plug);

#ifdef CONFIG_HMFS_CHECK_FS
		blkaddr = bio->bi_start_blkaddr;
		se = get_seg_entry(sbi, GET_SEGNO(sbi, blkaddr));
		if (type == DATA)
			f2fs_bug_on(sbi, !IS_DATASEG(se->type));
		else if (type == NODE)
			f2fs_bug_on(sbi, !IS_NODESEG(se->type));
#endif

		start = bio->bi_iter.bi_size >> F2FS_BLKSIZE_BITS;
		start %= F2FS_IO_SIZE(sbi);

		if (start == 0)
			goto submit_io;

		/* fill dummy pages */
		for (; start < F2FS_IO_SIZE(sbi); start++) {
			struct page *page =
				mempool_alloc(sbi->write_io_dummy,
					GFP_NOIO | __GFP_ZERO | __GFP_NOFAIL);
			f2fs_bug_on(sbi, !page);

			/* dummy page feature is never used */
			f2fs_bug_on(sbi, 1);

			SetPagePrivate(page);/*lint !e527*/
			set_page_private(page, (unsigned long)DUMMY_WRITTEN_PAGE);
			lock_page(page);
			if (bio_add_page(bio, page, PAGE_SIZE, 0) < PAGE_SIZE)
				f2fs_bug_on(sbi, 1);
		}
		/*
		 * In the NODE case, we lose next block address chain. So, we
		 * need to do checkpoint in hmfs_sync_file.
		 */
		if (type == NODE)
			set_sbi_flag(sbi, SBI_NEED_CP);
	}
submit_io:

	if (is_read_io(bio_op(bio))) {
		trace_hmfs_submit_read_bio(sbi->sb, type, bio);
		submit_bio(bio);
	} else {
#ifdef CONFIG_MAS_BLK
		if (IS_HMFS_GC_THREAD() && sbi->gc_control_info.iolimit > 0)
			hmfs_gc_wait(sbi);
#endif

		trace_hmfs_submit_write_bio(sbi->sb, type, bio);
		__hmfs_submit_write_bio(sbi, bio, type);
	}
}

#ifdef CONFIG_HMFS_CHECK_FS
static void hmfs_check_bio_crossed(struct f2fs_bio_info *io)
{
	struct f2fs_io_info *fio = &io->fio;
	struct f2fs_sb_info *sbi = io->sbi;
	struct bio *bio = io->bio;
	unsigned int start_blkaddr, end_blkaddr;
	unsigned int bio_pages = io->bio->bi_iter.bi_size / PAGE_SIZE;
	int i;

	if (fio->type != DATA && fio->type != NODE)
		return;

	if (!sbi->s_ndevs) {
		start_blkaddr = 0;
	} else {
		for (i = 0; i < sbi->s_ndevs; i++) {
			if (__hmfs_same_bdev(sbi, FDEV(i).start_blk, bio)) {
				start_blkaddr = FDEV(i).start_blk;
				break;
			}
		}

		if (i >= sbi->s_ndevs)
			return;
	}

	start_blkaddr +=  SECTOR_TO_BLOCK(bio->bi_iter.bi_sector);
	f2fs_bug_on(sbi, fio->new_blkaddr != start_blkaddr);

	if (bio_pages <= 1) {
		return;                                 /* no need to check */
	}

	end_blkaddr = start_blkaddr + bio_pages - 1;
	if (end_blkaddr >= MAX_BLKADDR(sbi))	{
		trace_printk("bio_blkaddr %u fio->newaddr %u blks %u %u\n",
					start_blkaddr, fio->new_blkaddr, bio_pages, end_blkaddr);
		f2fs_bug_on(sbi, 1);
	} else if (GET_SEC_FROM_SEG(sbi, GET_SEGNO(sbi, start_blkaddr)) !=
			GET_SEC_FROM_SEG(sbi, GET_SEGNO(sbi, end_blkaddr))) {
		trace_printk("bio_blkaddr %u fio->newaddr %u blks %u %u %u\n",
					start_blkaddr, fio->new_blkaddr, bio_pages,
			GET_SEC_FROM_SEG(sbi, GET_SEGNO(sbi, start_blkaddr)),
			GET_SEC_FROM_SEG(sbi, GET_SEGNO(sbi, end_blkaddr)));
		f2fs_bug_on(sbi, 1);
	}
}
#else
static void hmfs_check_bio_crossed(struct f2fs_bio_info *io)
{
	return;
}
#endif

static void __submit_merged_bio(struct f2fs_bio_info *io)
{
	struct f2fs_io_info *fio = &io->fio;

	if (!io->bio)
		return;

	bio_set_op_attrs(io->bio, fio->op, fio->op_flags);

	if (is_read_io(fio->op))
		trace_hmfs_prepare_read_bio(io->sbi->sb, fio->type, io->bio);
	else {
		hmfs_check_bio_crossed(io);
		trace_hmfs_prepare_write_bio(io->sbi->sb, fio->type, io->bio);
	}

	hmfs_bio_set_stream(io->sbi, io->bio, fio->type, fio->temp);
	/* must after hmfs_bio_set_stream */
	bio_set_oob(io->sbi, io->bio, fio->type, fio->ino, fio->page->index);
	hmfs_bio_set_flash_mode(io->sbi, io->bio, fio->new_blkaddr,
			io->bio->mas_bio.stream_type);

#ifdef CONFIG_MAS_BLK
	if (IS_HMFS_GC_THREAD())
		io->bio->mas_bi_opf |= MAS_REQ_GC;
#endif

	__submit_bio(io->sbi, io->bio, fio->type);

#ifdef CONFIG_MAS_BLK
	if (IS_HMFS_GC_THREAD() && !is_read_io(fio->op))
		atomic_inc(&(fio->sbi->gc_control_info.inflight));
#endif

	io->bio = NULL;
}

static bool __has_merged_page(struct f2fs_bio_info *io, struct inode *inode,
						struct page *page, nid_t ino)
{
	struct bio_vec *bvec;
	struct page *target;
	int i;

	if (!io->bio)
		return false;

	if (!inode && !page && !ino)
		return true;

	bio_for_each_segment_all(bvec, io->bio, i) {

		if (bvec->bv_page->mapping)
			target = bvec->bv_page;
		else
			target = fscrypt_control_page(bvec->bv_page);
		if (inode && inode == target->mapping->host)
			return true;
		if (page && page == target)
			return true;
		if (ino && ino == ino_of_node(target))
			return true;
	}

	return false;
}

static bool has_merged_page(struct f2fs_sb_info *sbi, struct inode *inode,
						struct page *page, nid_t ino,
						enum page_type type)
{
	enum page_type btype = PAGE_TYPE_OF_BIO(type);
	enum temp_type temp;
	struct f2fs_bio_info *io;
	bool ret = false;

	for (temp = HOT; temp < NR_TEMP_TYPE; temp++) {
		io = sbi->write_io[btype] + temp;

		down_read(&io->io_rwsem);
		ret = __has_merged_page(io, inode, page, ino);
		up_read(&io->io_rwsem);

		/* TODO: use HOT temp only for meta pages now. */
		if (ret || btype == META)
			break;
	}
	return ret;
}

static void __hmfs_submit_merged_write(struct f2fs_sb_info *sbi,
				enum page_type type, enum temp_type temp)
{
	enum page_type btype = PAGE_TYPE_OF_BIO(type);
	struct f2fs_bio_info *io = sbi->write_io[btype] + temp;

	down_write(&io->io_rwsem);

	/* change META to META_FLUSH in the checkpoint procedure */
	if (type >= META_FLUSH) {
		io->fio.type = META_FLUSH;
		io->fio.op = REQ_OP_WRITE;
		io->fio.op_flags = REQ_META | REQ_PRIO | REQ_SYNC;
		if (!test_opt(sbi, NOBARRIER))
			io->fio.op_flags |= REQ_PREFLUSH | REQ_FUA;
	}
	__submit_merged_bio(io);
	up_write(&io->io_rwsem);
}

static void __submit_merged_write_cond(struct f2fs_sb_info *sbi,
				struct inode *inode, struct page *page,
				nid_t ino, enum page_type type, bool force)
{
	enum temp_type temp;

	if (!force && !has_merged_page(sbi, inode, page, ino, type))
		return;

	for (temp = HOT; temp < NR_TEMP_TYPE; temp++) {

		__hmfs_submit_merged_write(sbi, type, temp);

		/* TODO: use HOT temp only for meta pages now. */
		if (type >= META)
			break;
	}
}

void hmfs_submit_merged_write(struct f2fs_sb_info *sbi, enum page_type type)
{
	__submit_merged_write_cond(sbi, NULL, 0, 0, type, true);
}

void hmfs_submit_merged_write_cond(struct f2fs_sb_info *sbi,
				struct inode *inode, struct page *page,
				nid_t ino, enum page_type type)
{
	__submit_merged_write_cond(sbi, inode, page, ino, type, false);
}

void hmfs_flush_merged_writes(struct f2fs_sb_info *sbi)
{
	hmfs_submit_merged_write(sbi, DATA);
	hmfs_submit_merged_write(sbi, NODE);
	hmfs_submit_merged_write(sbi, META);
}

/*
 * Fill the locked page with data located in the block address.
 * A caller needs to unlock the page on failure.
 */
int hmfs_submit_page_bio(struct f2fs_io_info *fio)
{
	struct bio *bio;
	struct page *page = fio->encrypted_page ?
			fio->encrypted_page : fio->page;
	enum count_type type = is_read_io(fio->op) ?
			__read_io_type(page): WB_DATA_TYPE(fio->page);

	if (!hmfs_is_valid_blkaddr(fio->sbi, fio->new_blkaddr,
			__is_meta_io(fio) ? META_GENERIC : DATA_GENERIC))
		return -EFAULT;

	trace_hmfs_submit_page_bio(page, fio);

	hmfs_fio_check(fio);

	f2fs_trace_ios(fio, 0);

	/* Allocate a new bio */
	bio = __bio_alloc(fio->sbi, fio->new_blkaddr, fio->io_wbc,
				1, is_read_io(fio->op), fio->type, fio->temp);

#ifdef CONFIG_HMFS_FS_ENCRYPTION
	if (fio->ci_key) {
		bio->ci_key = fio->ci_key;
		bio->ci_key_len = fio->ci_key_len;
		bio->ci_key_index = fio->ci_key_index;
		bio->ci_metadata = fio->ci_metadata;
		bio->index = page->index;
	}
#endif

	if (bio_add_page(bio, page, PAGE_SIZE, 0) < PAGE_SIZE) {
		bio_put(bio);
		return -EFAULT;
	}
	/*
	 * For storage turbo device, an opened stream corresponds to one section,
	 * one stream id can only correspond to one opened stream at a time, which
	 * means bio submission should not cross one section.
	 */
	if (!is_read_io(fio->op) &&
		IS_LAST_DATA_BLOCK_IN_SEC(fio->sbi, fio->new_blkaddr, fio->type))
		bio->bi_opf |= REQ_NOMERGE;

	if (fio->io_wbc && !is_read_io(fio->op))
		wbc_account_io(fio->io_wbc, page, PAGE_SIZE);

	bio_set_op_attrs(bio, fio->op, fio->op_flags);

	inc_page_count(fio->sbi, type);

	if (!is_read_io(fio->op))
		set_page_wb_type(fio->page, type == F2FS_WB_CP_DATA);

	hmfs_bio_set_stream(fio->sbi, bio, fio->type, fio->temp);
	/* must after hmfs_bio_set_stream */
	bio_set_oob(fio->sbi, bio, fio->type, fio->ino, fio->page->index);
	hmfs_bio_set_flash_mode(fio->sbi, bio, fio->new_blkaddr,
			bio->mas_bio.stream_type);

	hmfs_datamove_check_mapped_blk(fio->sbi, bio, fio->new_blkaddr);
	__submit_bio(fio->sbi, bio, fio->type);
	return 0;
}

static bool check_oob_merge(struct f2fs_bio_info *io,
		struct f2fs_io_info *fio)
{
	struct f2fs_sb_info *sbi = fio->sbi;
	struct f2fs_inode_info *fi;
	struct page *bio_page;

	if (!hmfs_is_oob_enable(sbi))
		return true;

	/*
	 * only set oob info at DATA stream
	 */
	if (fio->type == NODE)
		return true;

	bio_page = fio->encrypted_page ? fio->encrypted_page : fio->page;
	/*
	 * UFS only carries 1 pair {ino, index} when w/o EHS support.
	 * In next generation chip w/ EHS support, it has no limits
	 * last_index_in_bio == index - 1
	 */
	if (io->fio.ino != fio->ino ||
			io->last_index_in_bio != bio_page->index - 1)
		return false;

	fi = F2FS_I(bio_page->mapping->host);
	if (fi->fsync_task == current && fi->fsync_atomic) {
		/* last atomic write should not be merged */
		if (!fi->oob_last_page)
			return true;
		if (io->last_index_in_bio == fi->oob_last_page->index - 1)
			return false;
		if (io->last_index_in_bio == fi->oob_last_page->index)
			return false;
	}

	return true;
}

/*
 * align bio's end address with pu size
 *
 * Before align:
 * -----------------------
 * | PU | ......... | PU |
 * -----------------------
 *   ^                ^
 *   |                |
 *   io_start         io_end
 *
 * After align:
 * -----------------------
 * | PU | ......... | PU |
 * -----------------------
 *   ^              ^
 *   |              |
 *   io_start       io_end
 */
static int align_fio_with_pu(struct f2fs_io_info *fio, int blocks)
{
	block_t start, end;
	struct f2fs_sb_info *sbi = fio->sbi;
	int stream_id = get_stream_id_by_type_temp(fio->type, fio->temp);

	if (__is_pu_align_enabled(sbi, CURSEG_T(stream_id))) {
		start = fio->new_blkaddr - MAIN_BLKADDR(sbi);
		end = start + blocks;

		if (end % sbi->pu_size[SLC_MODE])
			end -= (end % sbi->pu_size[SLC_MODE]);

		if (end > start)
			blocks = end - start;
	}

	return blocks;
}

void hmfs_submit_page_write(struct f2fs_io_info *fio)
{
	struct f2fs_sb_info *sbi = fio->sbi;
	enum page_type btype = PAGE_TYPE_OF_BIO(fio->type);
	struct f2fs_bio_info *io = sbi->write_io[btype] + fio->temp;
	struct page *bio_page;
	enum count_type type;

	f2fs_bug_on(sbi, is_read_io(fio->op));

	down_write(&io->io_rwsem);
next:
	if (fio->in_list) {
		spin_lock(&io->io_lock);
		if (list_empty(&io->io_list)) {
			spin_unlock(&io->io_lock);
			goto out;
		}
		fio = list_first_entry(&io->io_list,
						struct f2fs_io_info, list);
		list_del(&fio->list);
		spin_unlock(&io->io_lock);
	}

	if (__is_valid_data_blkaddr(fio->old_blkaddr))
		verify_block_addr(fio, fio->old_blkaddr);
	verify_block_addr(fio, fio->new_blkaddr);

	bio_page = fio->encrypted_page ? fio->encrypted_page : fio->page;

	/* set submitted = true as a return value */
	fio->submitted = true;

	type = WB_DATA_TYPE(bio_page);
	inc_page_count(sbi, type);
	set_page_wb_type(bio_page, type == F2FS_WB_CP_DATA);

	if (io->bio && (io->last_block_in_bio != fio->new_blkaddr - 1 ||
		(io->fio.op != fio->op || io->fio.op_flags != fio->op_flags) ||
		!__hmfs_same_bdev(sbi, fio->new_blkaddr, io->bio) ||
		IS_LAST_DATA_BLOCK_IN_SEC(sbi, io->last_block_in_bio, fio->type) ||
		!check_oob_merge(io, fio)))
		__submit_merged_bio(io);
#ifdef CONFIG_HMFS_FS_ENCRYPTION
	else if ((io->bio) && ((io->bio->ci_key != fio->ci_key) ||
				(io->bio->ci_key_len != fio->ci_key_len) ||
				(fio->ci_key && io->last_index_in_bio != bio_page->index - 1) ||
				(io->bio->ci_metadata != fio->ci_metadata)))
		__submit_merged_bio(io);
#endif

alloc_new:
	if (io->bio == NULL) {
		int bio_blocks = BIO_MAX_PAGES;
		bool nomerge = false;
		if ((fio->type == DATA || fio->type == NODE) &&
				fio->new_blkaddr & F2FS_IO_SIZE_MASK(sbi)) {
			dec_page_count(sbi, type);
			ClearPageCPdata(bio_page);
			fio->retry = true;
			goto skip;
		}

		/* if nomerge return true, bio would be tracked
		 * by wbt, we force to shrink the bio->bi_max_vecs,
		 * avoid too much delay introduced by a big bio.
		 */
		bio_blocks = wbt_max_bio_blocks(sbi->sb->s_bdev,
					fio->op | fio->op_flags,
					BIO_MAX_PAGES, &nomerge);
		bio_blocks = align_fio_with_pu(fio, bio_blocks);

		io->bio = __bio_alloc(sbi, fio->new_blkaddr, fio->io_wbc,
						bio_blocks, false,
						fio->type, fio->temp);

		if (nomerge)
			io->bio->bi_opf |= REQ_NOMERGE;

		io->fio = *fio;
#ifdef CONFIG_HMFS_FS_ENCRYPTION
		io->bio->ci_key = fio->ci_key;
		io->bio->ci_key_len = fio->ci_key_len;
		io->bio->ci_key_index = fio->ci_key_index;
		io->bio->ci_metadata = fio->ci_metadata;
		if (fio->ci_key)
			io->bio->index = bio_page->index;
#endif
	}
#ifdef CONFIG_HMFS_FS_ENCRYPTION
	f2fs_bug_on(sbi, (io->bio->ci_key != fio->ci_key) ||
			(io->bio->ci_key_len != fio->ci_key_len) ||
			(io->bio->ci_metadata != fio->ci_metadata));
#endif

	/*
	 * For storage turbo device, an opened stream corresponds to one section,
	 * one stream id can only correspond to one opened stream at a time, which
	 * means bio submission should not cross one section.
	 */
	if (IS_LAST_DATA_BLOCK_IN_SEC(sbi, fio->new_blkaddr, fio->type))
		io->bio->bi_opf |= REQ_NOMERGE;

	if (bio_add_page(io->bio, bio_page, PAGE_SIZE, 0) < PAGE_SIZE) {
		__submit_merged_bio(io);
		goto alloc_new;
	} else {
		/* if it's a wbt tracked bio, kick it once bio is full,
		 * so the wbt inflight could update in time.
		 */
		if ((io->bio->bi_opf & REQ_NOMERGE) &&
		     wbt_need_kick_bio(io->bio))
			__submit_merged_bio(io);
	}

	if (fio->io_wbc)
		wbc_account_io(fio->io_wbc, bio_page, PAGE_SIZE);

	io->last_block_in_bio = fio->new_blkaddr;
	io->last_index_in_bio = bio_page->index;
	f2fs_trace_ios(fio, 0);

	hmfs_fio_check(fio);

	trace_hmfs_submit_page_write(fio->page, fio);
skip:
	if (fio->in_list)
		goto next;
out:
	if (is_sbi_flag_set(sbi, SBI_IS_SHUTDOWN) ||
				f2fs_is_checkpoint_ready(sbi))
		__submit_merged_bio(io);
	up_write(&io->io_rwsem);
}

static struct bio *f2fs_grab_read_bio(struct inode *inode, block_t blkaddr,
			unsigned nr_pages, unsigned op_flag, struct page *page)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct bio *bio;
	struct bio_post_read_ctx *ctx;
	unsigned int post_read_steps = 0;
	bool need_key = false;

	if (!hmfs_is_valid_blkaddr(sbi, blkaddr, DATA_GENERIC))
		return ERR_PTR(-EFAULT);

	bio = f2fs_bio_alloc(sbi, min_t(int, nr_pages, BIO_MAX_PAGES), false);
	if (!bio)
		return ERR_PTR(-ENOMEM);
	hmfs_target_device(sbi, blkaddr, bio);
	bio->bi_end_io = f2fs_read_end_io;
	bio_set_op_attrs(bio, REQ_OP_READ, op_flag);

	if (f2fs_encrypted_file(inode))
		post_read_steps |= 1 << STEP_DECRYPT;
	if (post_read_steps) {
		if (!f2fs_inline_encrypted_inode(inode)) {
			ctx = mempool_alloc(bio_post_read_ctx_pool, GFP_NOFS);
			if (!ctx) {
				bio_put(bio);
				return ERR_PTR(-ENOMEM);
			}
			ctx->bio = bio;
			ctx->enabled_steps = post_read_steps;
			bio->bi_private = ctx;
		} else {
			if (!fscrypt_has_encryption_key(inode)) {
				bio_put(bio);
				return ERR_PTR(-ENOKEY);
			}
			need_key = true;
		}
	}
#ifdef CONFIG_HMFS_FS_ENCRYPTION
	if (need_key) {
		bio->ci_key = fscrypt_ci_key(inode);
		bio->ci_key_len = fscrypt_ci_key_len(inode);
		bio->ci_key_index = fscrypt_ci_key_index(inode);
		bio->ci_metadata = fscrypt_ci_metadata(inode);
		bio->index = page->index;
	}
#endif
	return bio;
}

/* This can handle encryption stuffs */
static int f2fs_submit_page_read(struct inode *inode, struct page *page,
							block_t blkaddr)
{
	struct bio *bio = f2fs_grab_read_bio(inode, blkaddr, 1, 0, page);

	if (IS_ERR(bio))
		return PTR_ERR(bio);

	/* wait for GCed page writeback via META_MAPPING */
	hmfs_wait_on_block_writeback(inode, blkaddr);

	if (bio_add_page(bio, page, PAGE_SIZE, 0) < PAGE_SIZE) {
		bio_put(bio);
		return -EFAULT;
	}
	ClearPageError(page);
	inc_page_count(F2FS_I_SB(inode), F2FS_RD_DATA);

	hmfs_datamove_check_mapped_blk(F2FS_I_SB(inode), bio, blkaddr);
	__submit_bio(F2FS_I_SB(inode), bio, DATA);
	return 0;
}

static void __set_data_blkaddr(struct dnode_of_data *dn)
{
	struct f2fs_node *rn = F2FS_NODE(dn->node_page);
	__le32 *addr_array;
	int base = 0;

	if (IS_INODE(dn->node_page) && f2fs_has_extra_attr(dn->inode))
		base = get_extra_isize(dn->inode);

	/* Get physical address of data block */
	addr_array = blkaddr_in_node(rn);
	addr_array[base + dn->ofs_in_node] = cpu_to_le32(dn->data_blkaddr);
}

/*
 * Lock ordering for the change of data block address:
 * ->data_page
 *  ->node_page
 *    update block addresses in the node page
 */
void hmfs_set_data_blkaddr(struct dnode_of_data *dn)
{
	hmfs_wait_on_page_writeback(dn->node_page, NODE, true);
	__set_data_blkaddr(dn);
	if (set_page_dirty(dn->node_page))
		dn->node_changed = true;
}

void hmfs_update_data_blkaddr(struct dnode_of_data *dn, block_t blkaddr)
{
	dn->data_blkaddr = blkaddr;
	hmfs_set_data_blkaddr(dn);
	hmfs_update_extent_cache(dn);
}

/* dn->ofs_in_node will be returned with up-to-date last block pointer */
int hmfs_reserve_new_blocks(struct dnode_of_data *dn, blkcnt_t count)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(dn->inode);
	int err;

	if (!count)
		return 0;

	if (unlikely(is_inode_flag_set(dn->inode, FI_NO_ALLOC)))
		return -EPERM;
	if (unlikely((err = inc_valid_block_count(sbi, dn->inode, &count))))
		return err;

	trace_hmfs_reserve_new_blocks(dn->inode, dn->nid,
						dn->ofs_in_node, count);

	hmfs_wait_on_page_writeback(dn->node_page, NODE, true);

	for (; count > 0; dn->ofs_in_node++) {
		block_t blkaddr = datablock_addr(dn->inode,
					dn->node_page, dn->ofs_in_node);
		if (blkaddr == NULL_ADDR) {
			dn->data_blkaddr = NEW_ADDR;
			__set_data_blkaddr(dn);
			count--;
		}
	}

	if (set_page_dirty(dn->node_page))
		dn->node_changed = true;
	return 0;
}

/* Should keep dn->ofs_in_node unchanged */
int hmfs_reserve_new_block(struct dnode_of_data *dn)
{
	unsigned int ofs_in_node = dn->ofs_in_node;
	int ret;

	ret = hmfs_reserve_new_blocks(dn, 1);
	dn->ofs_in_node = ofs_in_node;
	return ret;
}

int hmfs_reserve_block(struct dnode_of_data *dn, pgoff_t index)
{
	bool need_put = dn->inode_page ? false : true;
	int err;

	err = hmfs_get_dnode_of_data(dn, index, ALLOC_NODE);
	if (err)
		return err;

	if (dn->data_blkaddr == NULL_ADDR)
		err = hmfs_reserve_new_block(dn);
	if (err || need_put)
		f2fs_put_dnode(dn);
	return err;
}

int hmfs_get_block(struct dnode_of_data *dn, pgoff_t index)
{
	struct extent_info ei  = {0,0,0};
	struct inode *inode = dn->inode;

	if (hmfs_lookup_extent_cache(inode, index, &ei)) {
		dn->data_blkaddr = ei.blk + index - ei.fofs;
		return 0;
	}

	return hmfs_reserve_block(dn, index);
}

struct page *hmfs_get_read_data_page(struct inode *inode, pgoff_t index,
						int op_flags, bool for_write)
{
	struct address_space *mapping = inode->i_mapping;
	struct dnode_of_data dn;
	struct page *page;
	struct extent_info ei = {0,0,0};
	int err;

	page = f2fs_grab_cache_page(mapping, index, for_write);
	if (!page)
		return ERR_PTR(-ENOMEM);

	if (hmfs_lookup_extent_cache(inode, index, &ei)) {
		dn.data_blkaddr = ei.blk + index - ei.fofs;
		goto got_it;
	}

	set_new_dnode(&dn, inode, NULL, NULL, 0);
	err = hmfs_get_dnode_of_data(&dn, index, LOOKUP_NODE);
	if (err)
		goto put_err;
	f2fs_put_dnode(&dn);

	if (unlikely(dn.data_blkaddr == NULL_ADDR)) {
		err = -ENOENT;
		goto put_err;
	}
got_it:
	if (PageUptodate(page)) {
		unlock_page(page);
		return page;
	}

	/*
	 * A new dentry page is allocated but not able to be written, since its
	 * new inode page couldn't be allocated due to -ENOSPC.
	 * In such the case, its blkaddr can be remained as NEW_ADDR.
	 * see, f2fs_add_link -> hmfs_get_new_data_page ->
	 * hmfs_init_inode_metadata.
	 */
	if (dn.data_blkaddr == NEW_ADDR) {
		zero_user_segment(page, 0, PAGE_SIZE);
		if (!PageUptodate(page))
			SetPageUptodate(page);
		unlock_page(page);
		return page;
	}

	err = f2fs_submit_page_read(inode, page, dn.data_blkaddr);
	if (err)
		goto put_err;
	return page;

put_err:
	f2fs_put_page(page, 1);
	return ERR_PTR(err);
}

struct page *hmfs_find_data_page(struct inode *inode, pgoff_t index)
{
	struct address_space *mapping = inode->i_mapping;
	struct page *page;

	page = find_get_page(mapping, index);
	if (page && PageUptodate(page))
		return page;
	f2fs_put_page(page, 0);

	page = hmfs_get_read_data_page(inode, index, 0, false);
	if (IS_ERR(page))
		return page;

	if (PageUptodate(page))
		return page;

	wait_on_page_locked(page);
	if (unlikely(!PageUptodate(page))) {
		f2fs_put_page(page, 0);
		return ERR_PTR(-EIO);
	}
	return page;
}

/*
 * If it tries to access a hole, return an error.
 * Because, the callers, functions in dir.c and GC, should be able to know
 * whether this page exists or not.
 */
struct page *hmfs_get_lock_data_page(struct inode *inode, pgoff_t index,
							bool for_write)
{
	struct address_space *mapping = inode->i_mapping;
	struct page *page;
repeat:
	page = hmfs_get_read_data_page(inode, index, 0, for_write);
	if (IS_ERR(page))
		return page;

	/* wait for read completion */
	lock_page(page);
	if (unlikely(page->mapping != mapping)) {
		f2fs_put_page(page, 1);
		goto repeat;
	}
	if (unlikely(!PageUptodate(page))) {
		f2fs_put_page(page, 1);
		return ERR_PTR(-EIO);
	}
	return page;
}

/*
 * Caller ensures that this data page is never allocated.
 * A new zero-filled data page is allocated in the page cache.
 *
 * Also, caller should grab and release a rwsem by calling f2fs_lock_op() and
 * f2fs_unlock_op().
 * Note that, ipage is set only by make_empty_dir, and if any error occur,
 * ipage should be released by this function.
 */
struct page *hmfs_get_new_data_page(struct inode *inode,
		struct page *ipage, pgoff_t index, bool new_i_size)
{
	struct address_space *mapping = inode->i_mapping;
	struct page *page;
	struct dnode_of_data dn;
	int err;

	page = f2fs_grab_cache_page(mapping, index, true);
	if (!page) {
		/*
		 * before exiting, we should make sure ipage will be released
		 * if any error occur.
		 */
		f2fs_put_page(ipage, 1);
		return ERR_PTR(-ENOMEM);
	}

	set_new_dnode(&dn, inode, ipage, NULL, 0);
	err = hmfs_reserve_block(&dn, index);
	if (err) {
		f2fs_put_page(page, 1);
		return ERR_PTR(err);
	}
	if (!ipage)
		f2fs_put_dnode(&dn);

	if (PageUptodate(page))
		goto got_it;

	if (dn.data_blkaddr == NEW_ADDR) {
		zero_user_segment(page, 0, PAGE_SIZE);
		if (!PageUptodate(page))
			SetPageUptodate(page);
	} else {
		f2fs_put_page(page, 1);

		/* if ipage exists, blkaddr should be NEW_ADDR */
		f2fs_bug_on(F2FS_I_SB(inode), ipage);
		page = hmfs_get_lock_data_page(inode, index, true);
		if (IS_ERR(page))
			return page;
	}
got_it:
	if (new_i_size && i_size_read(inode) <
				((loff_t)(index + 1) << PAGE_SHIFT))
		f2fs_i_size_write(inode, ((loff_t)(index + 1) << PAGE_SHIFT));
	return page;
}

static int __allocate_data_block(struct dnode_of_data *dn, int seg_type, int contig_level)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(dn->inode);
	struct f2fs_summary sum;
	struct node_info ni;
	block_t old_blkaddr;
	blkcnt_t count = 1;
	int err;

	if (unlikely(is_inode_flag_set(dn->inode, FI_NO_ALLOC)))
		return -EPERM;

	err = f2fs_get_node_info(sbi, dn->nid, &ni);
	if (err)
		return err;

	dn->data_blkaddr = datablock_addr(dn->inode,
				dn->node_page, dn->ofs_in_node);
	if (dn->data_blkaddr != NULL_ADDR)
		goto alloc;

	if (unlikely((err = inc_valid_block_count(sbi, dn->inode, &count))))
		return err;

alloc:
	set_summary(&sum, dn->nid, dn->ofs_in_node, ni.version);
	old_blkaddr = dn->data_blkaddr;
	bd_mutex_lock(&sbi->bd_mutex);
	inc_bd_array_val(sbi, hotcold_cnt, HC_DIRECTIO, 1);
	bd_mutex_unlock(&sbi->bd_mutex);

	hmfs_allocate_data_block(sbi, NULL, old_blkaddr, &dn->data_blkaddr,
					&sum, seg_type, NULL, false, contig_level);
	if (GET_SEGNO(sbi, old_blkaddr) != NULL_SEGNO)
		invalidate_mapping_pages(META_MAPPING(sbi),
					old_blkaddr, old_blkaddr);
	hmfs_update_data_blkaddr(dn, dn->data_blkaddr);

	/*
	 * i_size will be updated by direct_IO. Otherwise, we'll get stale
	 * data from unwritten block via dio_read.
	 */
	return 0;
}

int hmfs_preallocate_blocks(struct kiocb *iocb, struct iov_iter *from)
{
	struct inode *inode = file_inode(iocb->ki_filp);
	struct f2fs_map_blocks map;
	int flag;
	int err = 0;
	bool direct_io = iocb->ki_flags & IOCB_DIRECT;

	/* Disable preallocate for Direct I/O*/
	if (direct_io)
		return err;

	if (is_inode_flag_set(inode, FI_NO_PREALLOC))
		return 0;

	map.m_lblk = F2FS_BLK_ALIGN(iocb->ki_pos);
	map.m_len = F2FS_BYTES_TO_BLK(iocb->ki_pos + iov_iter_count(from));
	if (map.m_len > map.m_lblk)
		map.m_len -= map.m_lblk;
	else
		map.m_len = 0;

	map.m_next_pgofs = NULL;
	map.m_next_extent = NULL;
	map.m_seg_type = NO_CHECK_TYPE;

	if (direct_io) {
		map.m_seg_type = hmfs_rw_hint_to_seg_type(F2FS_I_SB(inode), iocb->ki_hint);
		flag = f2fs_force_buffered_io(inode, iocb, from) ?
					F2FS_GET_BLOCK_PRE_AIO :
					F2FS_GET_BLOCK_PRE_DIO;
		goto map_blocks;
	}
	if (iocb->ki_pos + iov_iter_count(from) > MAX_INLINE_DATA(inode)) {
		err = hmfs_convert_inline_inode(inode);
		if (err)
			return err;
	}
	if (f2fs_has_inline_data(inode))
		return err;

	flag = F2FS_GET_BLOCK_PRE_AIO;

map_blocks:
	err = hmfs_map_blocks(inode, &map, 1, flag);
	if (map.m_len > 0 && err == -ENOSPC) {
		if (!direct_io)
			set_inode_flag(inode, FI_NO_PREALLOC);
		err = 0;
	}
	return err;
}

/* false - lock op, true - node changes */
bool __hmfs_do_map_lock(struct f2fs_sb_info *sbi, int flag,
		   bool lock, bool lock2)
{
	if (flag == F2FS_GET_BLOCK_PRE_AIO) {
		if (lock) {
retry_get_lock:
			if (READ_ONCE(sbi->cp_rwsem_owner)) {
				f2fs_lock_op(sbi);
				return false;
			} else {
				down_read(&sbi->node_change);
				if (READ_ONCE(sbi->cp_rwsem_owner)) {
					up_read(&sbi->node_change);
					goto retry_get_lock;
				}
				return true;
			}
		} else if (lock2) {
			up_read(&sbi->node_change);
		} else  {
			f2fs_unlock_op(sbi);
		}
		return false;
	} else {
		if (lock)
			f2fs_lock_op(sbi);
		else
			f2fs_unlock_op(sbi);
		return false;
	}
}

/*
 * hmfs_map_blocks() now supported readahead/bmap/rw direct_IO with
 * f2fs_map_blocks structure.
 * If original data blocks are allocated, then give them to blockdev.
 * Otherwise,
 *     a. preallocate requested block addresses
 *     b. do not use extent cache for better performance
 *     c. give the block addresses to blockdev
 */
int hmfs_map_blocks(struct inode *inode, struct f2fs_map_blocks *map,
						int create, int flag)
{
	unsigned int maxblocks = map->m_len;
	struct dnode_of_data dn;
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	int mode = create ? ALLOC_NODE : LOOKUP_NODE;
	pgoff_t pgofs, end_offset, end;
	int err = 0, ofs = 1;
	unsigned int ofs_in_node, last_ofs_in_node;
	blkcnt_t prealloc;
	struct extent_info ei = {0,0,0};
	block_t blkaddr;
	unsigned int start_pgofs;
	int contig_level;
	bool lock2 = false;
	bool io_order_locked = false;
	bool lfs_mode = test_opt(sbi, LFS);

	if (!maxblocks)
		return 0;

	contig_level = check_io_seq(maxblocks);
	map->m_len = 0;
	map->m_flags = 0;

	/* it only supports block size == page size */
	pgofs =	(pgoff_t)map->m_lblk;
	end = pgofs + maxblocks;

	if (!create && hmfs_lookup_extent_cache(inode, pgofs, &ei)) {
		map->m_pblk = ei.blk + pgofs - ei.fofs;
		map->m_len = min((pgoff_t)maxblocks, ei.fofs + ei.len - pgofs);
		map->m_flags = F2FS_MAP_MAPPED;
		if (map->m_next_extent)
			*map->m_next_extent = pgofs + map->m_len;

		/* for hardware encryption, but to avoid potential issue in future */
		if (flag == F2FS_GET_BLOCK_DIO)
			hmfs_wait_on_block_writeback_range(inode,
						map->m_pblk, map->m_len);
		goto out;
	}

	if (create) {
		lock2 = __hmfs_do_map_lock(sbi, flag, true, false);
	}

next_dnode:
	/* When reading holes, we need its node page */
	set_new_dnode(&dn, inode, NULL, NULL, 0);
	err = hmfs_get_dnode_of_data(&dn, pgofs, mode);
	if (err) {
		if (flag == F2FS_GET_BLOCK_BMAP)
			map->m_pblk = 0;
		if (err == -ENOENT) {
			err = 0;
			if (map->m_next_pgofs)
				*map->m_next_pgofs =
					hmfs_get_next_page_offset(&dn, pgofs);
			if (map->m_next_extent)
				*map->m_next_extent =
					hmfs_get_next_page_offset(&dn, pgofs);
		}
		goto unlock_out;
	}

	start_pgofs = pgofs;
	prealloc = 0;
	last_ofs_in_node = ofs_in_node = dn.ofs_in_node;
	end_offset = ADDRS_PER_PAGE(dn.node_page, inode);

next_block:
	if (!io_order_locked && create && lfs_mode && flag == F2FS_GET_BLOCK_DIO) {
		down_write(&sbi->io_order_lock);
		io_order_locked = true;
	}

	blkaddr = datablock_addr(dn.inode, dn.node_page, dn.ofs_in_node);
	if (__is_valid_data_blkaddr(blkaddr) &&
		!hmfs_is_valid_blkaddr(sbi, blkaddr, DATA_GENERIC)) {
		err = -EFAULT;
		goto sync_out;
	}

	if (is_valid_data_blkaddr(sbi, blkaddr)) {
		/* use out-place-update for driect IO under LFS mode */
		if (test_opt(sbi, LFS) && create &&
				flag == F2FS_GET_BLOCK_DIO) {
			err = __allocate_data_block(&dn, map->m_seg_type, contig_level);
			if (err)
				goto sync_out;
			blkaddr = dn.data_blkaddr;
			if (!hmfs_is_oob_enable(sbi))
				set_inode_flag(inode, FI_APPEND_WRITE);
		}
	} else {
		if (create) {
			if (unlikely(f2fs_cp_error(sbi))) {
				err = -EIO;
				goto sync_out;
			}
			if (flag == F2FS_GET_BLOCK_PRE_AIO) {
				if (blkaddr == NULL_ADDR) {
					prealloc++;
					last_ofs_in_node = dn.ofs_in_node;
				}
			} else {
				WARN_ON(flag != F2FS_GET_BLOCK_PRE_DIO &&
					flag != F2FS_GET_BLOCK_DIO);
				err = __allocate_data_block(&dn,
					map->m_seg_type, contig_level);
				if (!err && !hmfs_is_oob_enable(sbi))
					set_inode_flag(inode, FI_APPEND_WRITE);
			}
			if (err)
				goto sync_out;
			map->m_flags |= F2FS_MAP_NEW;
			blkaddr = dn.data_blkaddr;
		} else {
			if (flag == F2FS_GET_BLOCK_BMAP) {
				map->m_pblk = 0;
				goto sync_out;
			}
			if (flag == F2FS_GET_BLOCK_PRECACHE)
				goto sync_out;
			if (flag == F2FS_GET_BLOCK_FIEMAP &&
						blkaddr == NULL_ADDR) {
				if (map->m_next_pgofs)
					*map->m_next_pgofs = pgofs + 1;
				goto sync_out;
			}
			if (flag != F2FS_GET_BLOCK_FIEMAP) {
				/* for defragment case */
				if (map->m_next_pgofs)
					*map->m_next_pgofs = pgofs + 1;
				goto sync_out;
			}
		}
	}

	if (flag == F2FS_GET_BLOCK_PRE_AIO)
		goto skip;

	if (map->m_len == 0) {
		/* preallocated unwritten block should be mapped for fiemap. */
		if (blkaddr == NEW_ADDR)
			map->m_flags |= F2FS_MAP_UNWRITTEN;
		map->m_flags |= F2FS_MAP_MAPPED;

		map->m_pblk = blkaddr;
		map->m_len = 1;
	} else if ((map->m_pblk != NEW_ADDR &&
			blkaddr == (map->m_pblk + ofs)) ||
			(map->m_pblk == NEW_ADDR && blkaddr == NEW_ADDR) ||
			flag == F2FS_GET_BLOCK_PRE_DIO) {
		ofs++;
		map->m_len++;
	} else {
		goto sync_out;
	}

	if (create && blkaddr != NEW_ADDR &&
		IS_LAST_DATA_BLOCK_IN_SEC(sbi, blkaddr, DATA))
		goto sync_out;
skip:
	dn.ofs_in_node++;
	pgofs++;

	/* preallocate blocks in batch for one dnode page */
	if (flag == F2FS_GET_BLOCK_PRE_AIO &&
			(pgofs == end || dn.ofs_in_node == end_offset)) {

		dn.ofs_in_node = ofs_in_node;
		err = hmfs_reserve_new_blocks(&dn, prealloc);
		if (err)
			goto sync_out;

		map->m_len += dn.ofs_in_node - ofs_in_node;
		if (prealloc && dn.ofs_in_node != last_ofs_in_node + 1) {
			err = -ENOSPC;
			goto sync_out;
		}
		dn.ofs_in_node = end_offset;
	}

	if (pgofs >= end)
		goto sync_out;
	else if (dn.ofs_in_node < end_offset)
		goto next_block;

	if (flag == F2FS_GET_BLOCK_PRECACHE) {
		if (map->m_flags & F2FS_MAP_MAPPED) {
			unsigned int ofs = start_pgofs - map->m_lblk;

			hmfs_update_extent_cache_range(&dn,
				start_pgofs, map->m_pblk + ofs,
				map->m_len - ofs);
		}
	}

	if (!io_order_locked) {
		f2fs_put_dnode(&dn);
		goto next_dnode;
	}

sync_out:

	/* for hardware encryption, but to avoid potential issue in future */
	if (flag == F2FS_GET_BLOCK_DIO && map->m_flags & F2FS_MAP_MAPPED)
		hmfs_wait_on_block_writeback_range(inode,
						map->m_pblk, map->m_len);

	if (flag == F2FS_GET_BLOCK_PRECACHE) {
		if (map->m_flags & F2FS_MAP_MAPPED) {
			unsigned int ofs = start_pgofs - map->m_lblk;

			hmfs_update_extent_cache_range(&dn,
				start_pgofs, map->m_pblk + ofs,
				map->m_len - ofs);
		}
		if (map->m_next_extent)
			*map->m_next_extent = pgofs + 1;
	}
	f2fs_put_dnode(&dn);
unlock_out:
	if (create) {
		if (io_order_locked)
			up_write(&sbi->io_order_lock);
		(void)__hmfs_do_map_lock(sbi, flag, false, lock2);

		if (!lfs_mode || flag != F2FS_GET_BLOCK_DIO)
			hmfs_balance_fs(sbi, dn.node_changed);
	}
out:
	trace_hmfs_map_blocks(inode, map, err);
	return err;
}

bool hmfs_overwrite_io(struct inode *inode, loff_t pos, size_t len)
{
	struct f2fs_map_blocks map;
	block_t last_lblk;
	int err;

	if (pos + len > i_size_read(inode))
		return false;

	map.m_lblk = F2FS_BYTES_TO_BLK(pos);
	map.m_next_pgofs = NULL;
	map.m_next_extent = NULL;
	map.m_seg_type = NO_CHECK_TYPE;
	last_lblk = F2FS_BLK_ALIGN(pos + len);

	while (map.m_lblk < last_lblk) {
		map.m_len = last_lblk - map.m_lblk;
		err = hmfs_map_blocks(inode, &map, 0, F2FS_GET_BLOCK_DEFAULT);
		if (err || map.m_len == 0)
			return false;
		map.m_lblk += map.m_len;
	}
	return true;
}

static int __get_data_block(struct inode *inode, sector_t iblock,
			struct buffer_head *bh, int create, int flag,
			pgoff_t *next_pgofs, int seg_type)
{
	struct f2fs_map_blocks map;
	int err;

	map.m_lblk = iblock;
	map.m_len = bh->b_size >> inode->i_blkbits;
	map.m_next_pgofs = next_pgofs;
	map.m_next_extent = NULL;
	map.m_seg_type = seg_type;

	err = hmfs_map_blocks(inode, &map, create, flag);
	if (!err) {
		map_bh(bh, inode->i_sb, map.m_pblk);
		bh->b_state = (bh->b_state & ~F2FS_MAP_FLAGS) | map.m_flags;
		bh->b_size = (u64)map.m_len << inode->i_blkbits;
	}
	return err;
}

static int get_data_block(struct inode *inode, sector_t iblock,
			struct buffer_head *bh_result, int create, int flag,
			pgoff_t *next_pgofs)
{
	return __get_data_block(inode, iblock, bh_result, create,
				flag, next_pgofs,
				NO_CHECK_TYPE);
}

static int get_data_block_dio(struct inode *inode, sector_t iblock,
			struct buffer_head *bh_result, int create)
{
	int err;
	unsigned long max_size = BIO_MAX_PAGES << PAGE_SHIFT;

	if (bh_result->b_size > max_size)
		bh_result->b_size = max_size;

	err = __get_data_block(inode, iblock, bh_result, create,
				F2FS_GET_BLOCK_DIO, NULL,
				hmfs_rw_hint_to_seg_type(F2FS_I_SB(inode), inode->i_write_hint));
	/*
	 * To avoid hungtask:
	 * DIO:                                | CP:
	 * get addr A                          |
	 * merge into BIO, but not submit      |
	 *                                     | sync data(get addr A+1),
	 *                                     | wait IO writeback timeout,
	 *                                     | because IO(A) no submit
	 * get addr A+2 fail, because cp_rwsem |
	 */
	set_buffer_boundary(bh_result);

	return err;
}

static int get_data_block_bmap(struct inode *inode, sector_t iblock,
			struct buffer_head *bh_result, int create)
{
	/* Block number less than F2FS MAX BLOCKS */
	if (unlikely(iblock >= F2FS_I_SB(inode)->max_file_blocks))
		return -EFBIG;

	return __get_data_block(inode, iblock, bh_result, create,
				F2FS_GET_BLOCK_BMAP, NULL,
				NO_CHECK_TYPE);
}

static inline sector_t logical_to_blk(struct inode *inode, loff_t offset)
{
	return (offset >> inode->i_blkbits);
}

static inline loff_t blk_to_logical(struct inode *inode, sector_t blk)
{
	return (blk << inode->i_blkbits);
}

static int f2fs_xattr_fiemap(struct inode *inode,
				struct fiemap_extent_info *fieinfo)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct page *page;
	struct node_info ni;
	__u64 phys = 0, len;
	__u32 flags;
	nid_t xnid = F2FS_I(inode)->i_xattr_nid;
	int err = 0;

	if (f2fs_has_inline_xattr(inode)) {
		int offset;

		page = f2fs_grab_cache_page(NODE_MAPPING(sbi),
						inode->i_ino, false);
		if (!page)
			return -ENOMEM;

		err = f2fs_get_node_info(sbi, inode->i_ino, &ni);
		if (err) {
			f2fs_put_page(page, 1);
			return err;
		}

		phys = (__u64)blk_to_logical(inode, ni.blk_addr);
		offset = offsetof(struct f2fs_inode, i_addr) +
					sizeof(__le32) * (DEF_ADDRS_PER_INODE -
					get_inline_xattr_addrs(inode));

		phys += offset;
		len = inline_xattr_size(inode);

		f2fs_put_page(page, 1);

		flags = FIEMAP_EXTENT_DATA_INLINE | FIEMAP_EXTENT_NOT_ALIGNED;

		if (!xnid)
			flags |= FIEMAP_EXTENT_LAST;

		err = fiemap_fill_next_extent(fieinfo, 0, phys, len, flags);
		if (err || err == 1)
			return err;
	}

	if (xnid) {
		page = f2fs_grab_cache_page(NODE_MAPPING(sbi), xnid, false);
		if (!page)
			return -ENOMEM;

		err = f2fs_get_node_info(sbi, xnid, &ni);
		if (err) {
			f2fs_put_page(page, 1);
			return err;
		}

		phys = (__u64)blk_to_logical(inode, ni.blk_addr);
		len = inode->i_sb->s_blocksize;

		f2fs_put_page(page, 1);

		flags = FIEMAP_EXTENT_LAST;
	}

	if (phys)
		err = fiemap_fill_next_extent(fieinfo, 0, phys, len, flags);

	return (err < 0 ? err : 0);
}

int hmfs_fiemap(struct inode *inode, struct fiemap_extent_info *fieinfo,
		u64 start, u64 len)
{
	struct buffer_head map_bh;
	sector_t start_blk, last_blk;
	pgoff_t next_pgofs;
	u64 logical = 0, phys = 0, size = 0;
	u32 flags = 0;
	int ret = 0;

	if (fieinfo->fi_flags & FIEMAP_FLAG_CACHE) {
		ret = hmfs_precache_extents(inode);
		if (ret)
			return ret;
	}

	ret = fiemap_check_flags(fieinfo, FIEMAP_FLAG_SYNC | FIEMAP_FLAG_XATTR);
	if (ret)
		return ret;

	inode_lock(inode);

	if (fieinfo->fi_flags & FIEMAP_FLAG_XATTR) {
		ret = f2fs_xattr_fiemap(inode, fieinfo);
		goto out;
	}

	if (f2fs_has_inline_data(inode)) {
		ret = hmfs_inline_data_fiemap(inode, fieinfo, start, len);
		if (ret != -EAGAIN)
			goto out;
	}

	if (logical_to_blk(inode, len) == 0)
		len = blk_to_logical(inode, 1);

	start_blk = logical_to_blk(inode, start);
	last_blk = logical_to_blk(inode, start + len - 1);

next:
	memset(&map_bh, 0, sizeof(struct buffer_head));
	map_bh.b_size = len;

	ret = get_data_block(inode, start_blk, &map_bh, 0,
					F2FS_GET_BLOCK_FIEMAP, &next_pgofs);
	if (ret)
		goto out;

	/* HOLE */
	if (!buffer_mapped(&map_bh)) {
		start_blk = next_pgofs;

		if (blk_to_logical(inode, start_blk) < blk_to_logical(inode,
					F2FS_I_SB(inode)->max_file_blocks))
			goto prep_next;

		flags |= FIEMAP_EXTENT_LAST;
	}

	if (size) {
		if (f2fs_encrypted_inode(inode))
			flags |= FIEMAP_EXTENT_DATA_ENCRYPTED;

		ret = fiemap_fill_next_extent(fieinfo, logical,
				phys, size, flags);
	}

	if (start_blk > last_blk || ret)
		goto out;

	logical = blk_to_logical(inode, start_blk);
	phys = blk_to_logical(inode, map_bh.b_blocknr);
	size = map_bh.b_size;
	flags = 0;
	if (buffer_unwritten(&map_bh))
		flags = FIEMAP_EXTENT_UNWRITTEN;

	start_blk += logical_to_blk(inode, size);

prep_next:
	cond_resched();
	if (fatal_signal_pending(current))
		ret = -EINTR;
	else
		goto next;
out:
	if (ret == 1)
		ret = 0;

	inode_unlock(inode);
	return ret;
}

/*
 * This function was originally taken from fs/mpage.c, and customized for f2fs.
 * Major change was from block_size == page_size in f2fs by default.
 *
 * Note that the aops->readpages() function is ONLY used for read-ahead. If
 * this function ever deviates from doing just read-ahead, it should either
 * use ->readpage() or do the necessary surgery to decouple ->readpages()
 * from read-ahead.
 */
static int f2fs_mpage_readpages(struct address_space *mapping,
			struct list_head *pages, struct page *page,
			unsigned nr_pages, bool is_readahead)
{
	struct bio *bio = NULL;
	unsigned io_submited = 0;
	sector_t last_block_in_bio = 0;
	sector_t last_index_in_bio = 0;
	struct inode *inode = mapping->host;
	const unsigned blkbits = inode->i_blkbits;
	const unsigned blocksize = 1 << blkbits;
	sector_t block_in_file;
	sector_t last_block;
	sector_t last_block_in_file;
	sector_t block_nr;
	sector_t new_block_nr;
	bool blk_mapped = false;
	struct f2fs_map_blocks map;

	map.m_pblk = 0;
	map.m_lblk = 0;
	map.m_len = 0;
	map.m_flags = 0;
	map.m_next_pgofs = NULL;
	map.m_next_extent = NULL;
	map.m_seg_type = NO_CHECK_TYPE;

	if (pages)
		/*
		 * Get one quota before read pages, when this ends,
		 * get the rest of quotas according to how many bios
		 * we submited in this routine.
		 */
		blk_throtl_get_quota(inode->i_sb->s_bdev, PAGE_SIZE,
				     msecs_to_jiffies(100), true);

	for (; nr_pages; nr_pages--) {
		if (pages) {
			page = list_last_entry(pages, struct page, lru);

			prefetchw(&page->flags);
			list_del(&page->lru);
			if (add_to_page_cache_lru(page, mapping,
						  page->index,
						  readahead_gfp_mask(mapping)))
				goto next_page;
		}

		block_in_file = (sector_t)page->index;
		last_block = block_in_file + nr_pages;
		last_block_in_file = (i_size_read(inode) + blocksize - 1) >>
								blkbits;
		if (last_block > last_block_in_file)
			last_block = last_block_in_file;

		/*
		 * Map blocks using the previous result first.
		 */
		if ((map.m_flags & F2FS_MAP_MAPPED) &&
				block_in_file > map.m_lblk &&
				block_in_file < (map.m_lblk + map.m_len))
			goto got_it;

		/*
		 * Then do more hmfs_map_blocks() calls until we are
		 * done with this page.
		 */
		map.m_flags = 0;

		if (block_in_file < last_block) {
			map.m_lblk = block_in_file;
			map.m_len = last_block - block_in_file;

			if (hmfs_map_blocks(inode, &map, 0,
						F2FS_GET_BLOCK_DEFAULT))
				goto set_error_page;
		}
got_it:
		if ((map.m_flags & F2FS_MAP_MAPPED)) {
			block_nr = map.m_pblk + block_in_file - map.m_lblk;
			SetPageMappedToDisk(page);

			if (!PageUptodate(page) && !cleancache_get_page(page)) {
				SetPageUptodate(page);
				goto confused;
			}

			if (!hmfs_is_valid_blkaddr(F2FS_I_SB(inode), block_nr,
								DATA_GENERIC))
				goto set_error_page;
		} else {
			zero_user_segment(page, 0, PAGE_SIZE);
			if (!PageUptodate(page))
				SetPageUptodate(page);
			unlock_page(page);
			goto next_page;
		}

		new_block_nr = block_nr;

		blk_mapped = hmfs_datamove_get_mapped_blk(
				F2FS_I_SB(inode), new_block_nr,
				(block_t *)&block_nr);

		/*
		 * This page will go to BIO.  Do we need to send this
		 * BIO off first?
		 */
		if (bio && (last_block_in_bio != block_nr - 1 ||
			!__hmfs_same_bdev(F2FS_I_SB(inode), block_nr, bio) ||
			(f2fs_encrypted_inode(inode) &&
				f2fs_inline_encrypted_inode(inode) &&
				last_index_in_bio != block_in_file - 1))) {
submit_and_realloc:
			io_submited++;
			__submit_bio(F2FS_I_SB(inode), bio, DATA);
			bio = NULL;
		}
		if (bio == NULL) {
			bio = f2fs_grab_read_bio(inode, block_nr, nr_pages,
					is_readahead ? REQ_RAHEAD : 0, page);
			if (IS_ERR(bio)) {
				if (blk_mapped)
					hmfs_datamove_put_mapped_blk(F2FS_I_SB(inode));
				bio = NULL;
				goto set_error_page;
			}
		}

		/*
		 * If the page is under writeback, we need to wait for
		 * its completion to see the correct decrypted data.
		 */
		hmfs_wait_on_block_writeback(inode, block_nr);

		if (bio_add_page(bio, page, blocksize, 0) < blocksize)
			goto submit_and_realloc;
		if (blk_mapped)
			hmfs_datamove_add_to_bio(F2FS_I_SB(inode), bio);

		inc_page_count(F2FS_I_SB(inode), F2FS_RD_DATA);
		ClearPageError(page);
		last_block_in_bio = block_nr;
		last_index_in_bio = block_in_file;
		goto next_page;
set_error_page:
		SetPageError(page);
		zero_user_segment(page, 0, PAGE_SIZE);
		unlock_page(page);
		goto next_page;
confused:
		if (bio) {
			io_submited++;
			__submit_bio(F2FS_I_SB(inode), bio, DATA);
			bio = NULL;
		}
		unlock_page(page);
next_page:
		if (pages)
			put_page(page);
	}
	BUG_ON(pages && !list_empty(pages));
	if (bio) {
		io_submited++;
		__submit_bio(F2FS_I_SB(inode), bio, DATA);
	}

	if (io_submited)
		blk_throtl_get_quotas(inode->i_sb->s_bdev, PAGE_SIZE,
				     msecs_to_jiffies(100), true,
				     io_submited - 1);
	return 0;
}

static int f2fs_read_data_page(struct file *file, struct page *page)
{
	struct inode *inode = page->mapping->host;
	int ret = -EAGAIN;

	trace_hmfs_readpage(page, DATA);

	/* If the file has inline data, try to read it directly */
	if (f2fs_has_inline_data(inode))
		ret = hmfs_read_inline_data(inode, page);
	if (ret == -EAGAIN)
		ret = f2fs_mpage_readpages(page->mapping, NULL, page, 1, false);
	return ret;
}

static int f2fs_read_data_pages(struct file *file,
			struct address_space *mapping,
			struct list_head *pages, unsigned nr_pages)
{
	struct inode *inode = mapping->host;
	struct page *page = list_last_entry(pages, struct page, lru);

	trace_hmfs_readpages(inode, page, nr_pages);

	/* If the file has inline data, skip readpages */
	if (f2fs_has_inline_data(inode))
		return 0;

	return f2fs_mpage_readpages(mapping, pages, NULL, nr_pages, true);
}

static int encrypt_one_page(struct f2fs_io_info *fio)
{
	struct inode *inode = fio->page->mapping->host;
	struct page *mpage;
	gfp_t gfp_flags = GFP_NOFS;

	if (!f2fs_encrypted_file(inode))
		return 0;

	/* wait for GCed page writeback via META_MAPPING */
	hmfs_wait_on_block_writeback(inode, fio->old_blkaddr);

	if (!f2fs_inline_encrypted_inode(inode)) {
retry_encrypt:
		fio->encrypted_page = fscrypt_encrypt_page(inode, fio->page,
				PAGE_SIZE, 0, fio->page->index, gfp_flags);
		if (IS_ERR(fio->encrypted_page)) {
			/* flush pending IOs and wait for a while in the ENOMEM case */
			if (PTR_ERR(fio->encrypted_page) == -ENOMEM) {
				hmfs_flush_merged_writes(fio->sbi);
				congestion_wait(BLK_RW_ASYNC, HZ/50);
				gfp_flags |= __GFP_NOFAIL;
				goto retry_encrypt;
			}
			return PTR_ERR(fio->encrypted_page);
		}
		mpage = find_lock_page(META_MAPPING(fio->sbi), fio->old_blkaddr);
		if (mpage) {
			if (PageUptodate(mpage))
				memcpy(page_address(mpage),
					page_address(fio->encrypted_page), PAGE_SIZE);
			f2fs_put_page(mpage, 1);
		}
	} else {
		if (!fscrypt_has_encryption_key(inode)) {
			return -ENOKEY;
		}
#ifdef CONFIG_HMFS_FS_ENCRYPTION
		fio->ci_key = fscrypt_ci_key(inode);
		fio->ci_key_len = fscrypt_ci_key_len(inode);
		fio->ci_key_index = fscrypt_ci_key_index(inode);
		fio->ci_metadata = fscrypt_ci_metadata(inode);
#endif
	}
	return 0;
}

static inline bool check_inplace_update_policy(struct inode *inode,
				struct f2fs_io_info *fio)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	unsigned int policy = SM_I(sbi)->ipu_policy;

	if (policy & (0x1 << F2FS_IPU_FORCE))
		return true;
	if (policy & (0x1 << F2FS_IPU_SSR) && hmfs_need_SSR(sbi))
		return true;
	if (policy & (0x1 << F2FS_IPU_UTIL) &&
			utilization(sbi) > SM_I(sbi)->min_ipu_util)
		return true;
	if (policy & (0x1 << F2FS_IPU_SSR_UTIL) && hmfs_need_SSR(sbi) &&
			utilization(sbi) > SM_I(sbi)->min_ipu_util)
		return true;

	/*
	 * IPU for rewrite async pages
	 */
	if (policy & (0x1 << F2FS_IPU_ASYNC) &&
			fio && fio->op == REQ_OP_WRITE &&
			!(fio->op_flags & REQ_SYNC) &&
			!f2fs_encrypted_inode(inode))
		return true;

	/* this is only set during fdatasync */
	if (policy & (0x1 << F2FS_IPU_FSYNC) &&
			is_inode_flag_set(inode, FI_NEED_IPU))
		return true;

	if (unlikely(fio && is_sbi_flag_set(sbi, SBI_CP_DISABLED) &&
			!hmfs_is_checkpointed_data(sbi, fio->old_blkaddr)))
		return true;

	return false;
}

bool hmfs_should_update_inplace(struct inode *inode, struct f2fs_io_info *fio)
{
	if (f2fs_is_pinned_file(inode))
		return true;

	/* if this is cold file, we should overwrite to avoid fragmentation */
	if (file_is_cold(inode))
		return true;

	return check_inplace_update_policy(inode, fio);
}

bool hmfs_should_update_outplace(struct inode *inode, struct f2fs_io_info *fio)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);

	if (test_opt(sbi, LFS))
		return true;
	if (S_ISDIR(inode->i_mode))
		return true;
	if (IS_NOQUOTA(inode))
		return true;
	if (f2fs_is_atomic_file(inode))
		return true;
	if (fio) {
		if (is_cold_data(fio->page))
			return true;
		if (IS_ATOMIC_WRITTEN_PAGE(fio->page))
			return true;
		if (unlikely(is_sbi_flag_set(sbi, SBI_CP_DISABLED) &&
			hmfs_is_checkpointed_data(sbi, fio->old_blkaddr)))
			return true;
	}
	return false;
}

static inline bool need_inplace_update(struct f2fs_io_info *fio)
{
	struct inode *inode = fio->page->mapping->host;

	if (f2fs_is_pinned_file(inode))
		return true;

	if (hmfs_should_update_outplace(inode, fio))
		return false;

	return hmfs_should_update_inplace(inode, fio);
}

int hmfs_do_write_data_page(struct f2fs_io_info *fio)
{
	struct page *page = fio->page;
	struct inode *inode = page->mapping->host;
	struct dnode_of_data dn;
	struct extent_info ei = {0,0,0};
	struct node_info ni;
	bool ipu_force = false;
	int err = 0;
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);

	set_new_dnode(&dn, inode, NULL, NULL, 0);
	dn.mem_control = fio->mem_control;
	if (need_inplace_update(fio) &&
			hmfs_lookup_extent_cache(inode, page->index, &ei)) {
		fio->old_blkaddr = ei.blk + page->index - ei.fofs;

		if (!hmfs_is_valid_blkaddr(fio->sbi, fio->old_blkaddr,
							DATA_GENERIC))
			return -EFAULT;

		ipu_force = true;
		fio->need_lock = LOCK_DONE;
		goto got_it;
	}

	/* Deadlock due to between page->lock and f2fs_lock_op */
	if (fio->need_lock == LOCK_REQ && !f2fs_trylock_op(fio->sbi))
		return -EAGAIN;

	err = hmfs_get_dnode_of_data(&dn, page->index, LOOKUP_NODE);
	if (err)
		goto out;

	fio->old_blkaddr = dn.data_blkaddr;

	/* This page is already truncated */
	if (fio->old_blkaddr == NULL_ADDR) {
		ClearPageUptodate(page);
		clear_cold_data(page);
		goto out_writepage;
	}
got_it:
	if (__is_valid_data_blkaddr(fio->old_blkaddr) &&
		!hmfs_is_valid_blkaddr(fio->sbi, fio->old_blkaddr,
							DATA_GENERIC)) {
		err = -EFAULT;
		goto out_writepage;
	}
	/*
	 * If current allocation needs SSR,
	 * it had better in-place writes for updated data.
	 */
	if (ipu_force || (is_valid_data_blkaddr(fio->sbi, fio->old_blkaddr) &&
					need_inplace_update(fio))) {
		if (S_ISDIR(inode->i_mode)) {
			bd_mutex_lock(&sbi->bd_mutex);
			inc_bd_array_val(sbi, hotcold_cnt, HC_REWRITE_HOT_DATA, 1);
			bd_mutex_unlock(&sbi->bd_mutex);
		} else {
			bd_mutex_lock(&sbi->bd_mutex);
			inc_bd_array_val(sbi, hotcold_cnt, HC_REWRITE_WARM_DATA, 1);
			bd_mutex_unlock(&sbi->bd_mutex);
		}
		err = encrypt_one_page(fio);
		if (err)
			goto out_writepage;

		set_page_writeback(page);
		ClearPageError(page);
		f2fs_put_dnode(&dn);
		if (fio->need_lock == LOCK_REQ)
			f2fs_unlock_op(fio->sbi);
		err = hmfs_inplace_write_data(fio);
		if (err) {
			if (f2fs_encrypted_file(inode))
				fscrypt_pullback_bio_page(&fio->encrypted_page,
									true);
			if (PageWriteback(page))
				end_page_writeback(page);
		}
		trace_hmfs_do_write_data_page(fio->page, IPU);
		set_inode_flag(inode, FI_UPDATE_WRITE);
		return err;
	}

	if (fio->need_lock == LOCK_RETRY) {
		if (!f2fs_trylock_op(fio->sbi)) {
			err = -EAGAIN;
			goto out_writepage;
		}
		fio->need_lock = LOCK_REQ;
	}

	err = f2fs_get_node_info(fio->sbi, dn.nid, &ni);
	if (err)
		goto out_writepage;

	fio->version = ni.version;

	err = encrypt_one_page(fio);
	if (err)
		goto out_writepage;

	set_page_writeback(page);
	ClearPageError(page);

	/* LFS mode write path */
	err = hmfs_outplace_write_data(&dn, fio);
	if (err) {
		/*
		 * If write data page meets error,
		 * clear the writeback flag here
		 */
		end_page_writeback(page);
		goto out_writepage;
	}

	trace_hmfs_do_write_data_page(page, OPU);
	if (!hmfs_is_oob_enable(sbi))
		set_inode_flag(inode, FI_APPEND_WRITE);
	if (page->index == 0)
		set_inode_flag(inode, FI_FIRST_BLOCK_WRITTEN);
out_writepage:
	f2fs_put_dnode(&dn);
out:
	if (fio->need_lock == LOCK_REQ)
		f2fs_unlock_op(fio->sbi);
	return err;
}

static int __write_data_page(struct page *page, bool *submitted,
				struct writeback_control *wbc,
				enum iostat_type io_type)
{
	struct inode *inode = page->mapping->host;
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	loff_t i_size = i_size_read(inode);
	const pgoff_t end_index = ((unsigned long long) i_size)
							>> PAGE_SHIFT;
	loff_t psize = (page->index + 1) << PAGE_SHIFT;
	unsigned offset = 0;
	bool need_balance_fs = false;
	int err = 0;
	struct f2fs_io_info fio = {
		.sbi = sbi,
		.ino = inode->i_ino,
		.type = DATA,
		.op = REQ_OP_WRITE,
		.op_flags = wbc_to_write_flags(wbc),
		.old_blkaddr = NULL_ADDR,
		.page = page,
		.encrypted_page = NULL,
		.submitted = false,
		.need_lock = LOCK_RETRY,
		.io_type = io_type,
		.io_wbc = wbc,
		.mem_control = wbc->for_free_mem,
	};

	trace_hmfs_writepage(page, DATA);

	/* we should bypass data pages to proceed the kworkder jobs */
	if (unlikely(f2fs_cp_error(sbi))) {
		mapping_set_error(page->mapping, -EIO);
		/*
		 * don't drop any dirty dentry pages for keeping lastest
		 * directory structure.
		 */
		if (S_ISDIR(inode->i_mode))
			goto redirty_out;
		goto out;
	}

	if (unlikely(is_sbi_flag_set(sbi, SBI_POR_DOING)))
		goto redirty_out;

	if (page->index < end_index)
		goto write;

	/*
	 * If the offset is out-of-range of file size,
	 * this page does not have to be written to disk.
	 */
	offset = i_size & (PAGE_SIZE - 1);
	if ((page->index >= end_index + 1) || !offset)
		goto out;

	zero_user_segment(page, offset, PAGE_SIZE);
write:
	if (f2fs_is_drop_cache(inode))
		goto out;
	/* we should not write 0'th page having journal header */
	if (f2fs_is_volatile_file(inode) && (!page->index ||
			(!wbc->for_reclaim &&
			hmfs_available_free_memory(sbi, BASE_CHECK))))
		goto redirty_out;

	/* Dentry blocks are controlled by checkpoint */
	if (S_ISDIR(inode->i_mode)) {
		fio.need_lock = LOCK_DONE;
		err = hmfs_do_write_data_page(&fio);
		goto done;
	}

	if (!wbc->for_reclaim)
		need_balance_fs = true;
	else if (has_not_enough_free_secs(sbi, 0, 0))
		goto redirty_out;
	else
		set_inode_flag(inode, FI_HOT_DATA);

	err = -EAGAIN;
	if (f2fs_has_inline_data(inode)) {
		err = hmfs_write_inline_data_ex(inode, page,
					(fio.mem_control == 1));
		if (!err)
			goto out;
		else if (fio.mem_control == 1)
			goto redirty_out;
	}

	if (err == -EAGAIN) {
		err = hmfs_do_write_data_page(&fio);
		if (err == -EAGAIN) {
			fio.need_lock = LOCK_REQ;
			err = hmfs_do_write_data_page(&fio);
		}
	}

	if (err) {
		file_set_keep_isize(inode);
	} else {
		down_write(&F2FS_I(inode)->i_sem);
		if (F2FS_I(inode)->last_disk_size < psize)
			F2FS_I(inode)->last_disk_size = psize;
		up_write(&F2FS_I(inode)->i_sem);
	}

done:
	if (err && err != -ENOENT)
		goto redirty_out;

out:
	inode_dec_dirty_pages(inode);
	if (err) {
		ClearPageUptodate(page);
		clear_cold_data(page);
	}

	if (wbc->for_reclaim) {
		hmfs_submit_merged_write_cond(sbi, NULL, page, 0, DATA);
		clear_inode_flag(inode, FI_HOT_DATA);
		hmfs_remove_dirty_inode(inode);
		submitted = NULL;
	}

	unlock_page(page);
	if (!S_ISDIR(inode->i_mode) && !IS_NOQUOTA(inode))
		hmfs_balance_fs(sbi, need_balance_fs);

	if (unlikely(f2fs_cp_error(sbi))) {
		hmfs_submit_merged_write(sbi, DATA);
		submitted = NULL;
	}

	if (submitted) {
		*submitted = fio.submitted;
		if (*submitted && (F2FS_I(inode)->fsync_task != current))
			F2FS_I(inode)->has_wb = true;
	}

	return 0;

redirty_out:
	redirty_page_for_writepage(wbc, page);
	/*
	 * pageout() in MM traslates EAGAIN, so calls handle_write_error()
	 * -> mapping_set_error() -> set_bit(AS_EIO, ...).
	 * file_write_and_wait_range() will see EIO error, which is critical
	 * to return value of fsync() followed by atomic_write failure to user.
	 */
	if (!err || wbc->for_reclaim)
		return AOP_WRITEPAGE_ACTIVATE;
	unlock_page(page);
	return err;
}

static int f2fs_write_data_page(struct page *page,
					struct writeback_control *wbc)
{
	return __write_data_page(page, NULL, wbc, FS_DATA_IO);
}

#ifdef CONFIG_BLK_WBT
static void f2fs_wbt_blk_throtl(struct block_device *bdev,
				struct writeback_control *wbc)
{
	struct request_queue *q;

	if (unlikely(!bdev) || unlikely(!bdev->bd_disk))
		return;

	q = bdev_get_queue(bdev);
	if (unlikely(!q))
		return;

	if (!wbt_fs_get_quota(q, wbc))
		wbt_fs_wait(q, wbc);
}
#endif

static struct page *last_fsync_data(struct address_space *mapping)
{
	pgoff_t index;
	struct pagevec pvec;
	struct page *last_page = NULL;
	struct f2fs_sb_info *sbi = F2FS_M_SB(mapping);
	int nr_pages;
	int i;

	pagevec_init(&pvec, 0);
	index = 0;

	while ((nr_pages = pagevec_lookup_tag(&pvec, mapping, &index,
					PAGECACHE_TAG_TOWRITE))) {
		for (i = 0; i < nr_pages; i++) {
			struct page *page = pvec.pages[i];

			if (unlikely(f2fs_cp_error(sbi))) {
				f2fs_put_page(last_page, 0);
				pagevec_release(&pvec);
				return ERR_PTR(-EIO);
			}

			lock_page(page);

			if (unlikely(page->mapping != mapping)) {
continue_unlock:
				unlock_page(page);
				continue;
			}

			if (!PageDirty(page)) {
				/* someone wrote it for us */
				goto continue_unlock;
			}

			if (last_page)
				f2fs_put_page(last_page, 0);

			get_page(page);
			last_page = page;
			unlock_page(page);
		}
		pagevec_release(&pvec);
		cond_resched();
	}

	trace_hmfs_last_flush_data(mapping->host->i_ino, last_page);
	return last_page;
}

/*
 * This function was copied from write_cche_pages from mm/page-writeback.c.
 * The major change is making write step of cold data page separately from
 * warm/hot data page.
 */
static int f2fs_write_cache_pages(struct address_space *mapping,
					struct writeback_control *wbc,
					enum iostat_type io_type)
{
	int ret = 0;
	int done = 0;
	struct pagevec pvec;
	struct f2fs_sb_info *sbi = F2FS_M_SB(mapping);
	int nr_pages;
	pgoff_t uninitialized_var(writeback_index);
	pgoff_t index;
	pgoff_t end;		/* Inclusive */
	pgoff_t done_index;
	int cycled;
	int range_whole = 0;
	int tag;
	int nwritten = 0;
	struct inode *inode = mapping->host;
	struct f2fs_inode_info *fi = F2FS_I(inode);

	pagevec_init(&pvec, 0);

	if (get_dirty_pages(inode) <= SM_I(sbi)->min_hot_blocks)
		set_inode_flag(inode, FI_HOT_DATA);
	else
		clear_inode_flag(inode, FI_HOT_DATA);

	if (wbc->range_cyclic) {
		writeback_index = mapping->writeback_index; /* prev offset */
		index = writeback_index;
		if (index == 0)
			cycled = 1;
		else
			cycled = 0;
		end = -1;
	} else {
		index = wbc->range_start >> PAGE_SHIFT;
		end = wbc->range_end >> PAGE_SHIFT;
		if (wbc->range_start == 0 && wbc->range_end == LLONG_MAX)
			range_whole = 1;
		cycled = 1; /* ignore range_cyclic tests */
	}
	if (wbc->sync_mode == WB_SYNC_ALL || wbc->tagged_writepages)
		tag = PAGECACHE_TAG_TOWRITE;
	else
		tag = PAGECACHE_TAG_DIRTY;
retry:
	if (wbc->sync_mode == WB_SYNC_ALL || wbc->tagged_writepages)
		tag_pages_for_writeback(mapping, index, end);
	if (fi->fsync_task == current && fi->fsync_atomic)
		fi->oob_last_page = last_fsync_data(mapping);

	done_index = index;
	while (!done && (index <= end)) {
		int i;

		nr_pages = pagevec_lookup_range_tag(&pvec, mapping, &index, end,
				tag);
		if (nr_pages == 0)
			break;

#ifdef CONFIG_BLK_WBT
		f2fs_wbt_blk_throtl(mapping->host->i_sb->s_bdev, wbc);
#endif
		for (i = 0; i < nr_pages; i++) {
			struct page *page = pvec.pages[i];
			bool submitted = false;

			/* give a priority to WB_SYNC threads */
			if (atomic_read(&sbi->wb_sync_req[DATA]) &&
					wbc->sync_mode == WB_SYNC_NONE) {
				done = 1;
				break;
			}

			done_index = page->index;
retry_write:
			lock_page(page);

			if (unlikely(page->mapping != mapping)) {
continue_unlock:
				unlock_page(page);
				continue;
			}

			if (!PageDirty(page)) {
				/* someone wrote it for us */
				goto continue_unlock;
			}

			if (PageWriteback(page)) {
				if (wbc->sync_mode != WB_SYNC_NONE)
					hmfs_wait_on_page_writeback(page,
								DATA, true);
				else
					goto continue_unlock;
			}

			BUG_ON(PageWriteback(page));
			if (!clear_page_dirty_for_io(page))
				goto continue_unlock;

			ret = __write_data_page(page, &submitted, wbc, io_type);
			if (unlikely(ret)) {
				/*
				 * keep nr_to_write, since vfs uses this to
				 * get # of written pages.
				 */
				if (ret == AOP_WRITEPAGE_ACTIVATE) {
					unlock_page(page);
					ret = 0;
					continue;
				} else if (ret == -EAGAIN) {
					ret = 0;
					if (wbc->sync_mode == WB_SYNC_ALL) {
						cond_resched();
						congestion_wait(BLK_RW_ASYNC,
									HZ/50);
						goto retry_write;
					}
					continue;
				}
				done_index = page->index + 1;
				done = 1;
				break;
			} else if (submitted) {
				nwritten++;
			}

			if (--wbc->nr_to_write <= 0 &&
					wbc->sync_mode == WB_SYNC_NONE) {
				done = 1;
				break;
			}
		}
		pagevec_release(&pvec);
		cond_resched();
	}

	if (!cycled && !done) {
		cycled = 1;
		index = 0;
		end = writeback_index - 1;
		goto retry;
	}
	if (wbc->range_cyclic || (range_whole && wbc->nr_to_write > 0))
		mapping->writeback_index = done_index;

#ifdef CONFIG_HMFS_JOURNAL_APPEND
	if (nwritten && (!write_opt || !task_in_wb_thrd(current))) {
#else
	if (nwritten) {
#endif
		hmfs_submit_merged_write_cond(F2FS_M_SB(mapping), mapping->host,
								NULL, 0, DATA);
		if ((fi->fsync_task != current) && (io_type == FS_DATA_IO))
			fi->has_wb = true;
	}

	if (fi->oob_last_page) {
		put_page(fi->oob_last_page);
		fi->oob_last_page = NULL;
	}

	return ret;
}

static inline bool __should_serialize_io(struct inode *inode,
					struct writeback_control *wbc)
{
	if (!S_ISREG(inode->i_mode))
		return false;
	if (IS_NOQUOTA(inode))
		return false;
	if (wbc->sync_mode != WB_SYNC_ALL)
		return true;
	if (get_dirty_pages(inode) >= SM_I(F2FS_I_SB(inode))->min_seq_blocks)
		return true;
	return false;
}

#define DEF_SYSTEM_THROTTLE_COUNT	128	/* 512KB */
#define DEF_FILE_THROTTLE_COUNT		4	/* 16KB */
#define DEF_FILE_SKIP_THRESHOLD		64

/*
 * It's little different from __get_segment_type_4:
 * Skip to judge whether page is cold, because page only set cold
 * in gc flow.
 * Right now data-gc uses datamove command, so no one set page cold.
 */
static int get_data_segment_type(struct inode *inode)
{
	if (file_is_cold(inode))
		return CURSEG_COLD_DATA;

	if (S_ISDIR(inode->i_mode) || file_is_hot(inode) ||
			is_inode_flag_set(inode, FI_HOT_DATA) ||
			f2fs_is_atomic_file(inode) ||
			f2fs_is_volatile_file(inode))
		return CURSEG_HOT_DATA;
	return hmfs_rw_hint_to_seg_type(F2FS_I_SB(inode), inode->i_write_hint);
}

/*
 * Try to align IOs of cold data stream into PU size, but no guarantee.
 * Because flush no-pu-align IOs for data consistency or lost:
 * for example, writeback by fsync
 */
static bool __is_need_pu_align(struct inode *inode,
		struct writeback_control *wbc)
{
	struct f2fs_inode_info *fi = F2FS_I(inode);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);

	if (sbi->skip_pu_align)
		return false;

	if (!__is_pu_align_enabled(sbi, get_data_segment_type(inode)))
		return false;

	if (wbc->sync_mode != WB_SYNC_NONE) {
		if (fi->fsync_task == current)
			stat_inc_pu_info(sbi, PU_FLUSH_FSYNC, 1);
		else
			stat_inc_pu_info(sbi, PU_FLUSH_SYNC, 1);
		return false;
	}

	if (fi->skip_count[SKIP_PU_ALIGN] > DEF_FILE_SKIP_THRESHOLD) {
		stat_inc_pu_info(sbi, PU_FLUSH_SKIP, 1);
		return false;
	}

	return true;
}

static int __f2fs_write_data_pages(struct address_space *mapping,
						struct writeback_control *wbc,
						enum iostat_type io_type)
{
	struct inode *inode = mapping->host;
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct f2fs_inode_info *fi = F2FS_I(inode);
	struct blk_plug plug;
	int ret;
	bool locked = false;

	/* deal with chardevs and other special file */
	if (!mapping->a_ops->writepage)
		return 0;

	/* skip writing if there is no dirty page in this inode */
	if (!get_dirty_pages(inode) && wbc->sync_mode == WB_SYNC_NONE)
		return 0;

	/* during POR, we don't need to trigger writepage at all. */
	if (unlikely(is_sbi_flag_set(sbi, SBI_POR_DOING)))
		goto skip_write;

	if ((S_ISDIR(inode->i_mode) || IS_NOQUOTA(inode)) &&
			wbc->sync_mode == WB_SYNC_NONE &&
			get_dirty_pages(inode) < nr_pages_to_skip(sbi, DATA) &&
			hmfs_available_free_memory(sbi, DIRTY_DENTS))
		goto skip_write;
	if (is_inode_flag_set(inode, FI_LOG_FILE) &&
		wbc->sync_mode == WB_SYNC_NONE &&
		(get_dirty_pages(inode) <= DEF_FILE_THROTTLE_COUNT &&
		fi->skip_count[SKIP_LOG_FILE] <= DEF_FILE_SKIP_THRESHOLD) &&
		(get_pages(sbi, F2FS_LOG_FILE) <= DEF_SYSTEM_THROTTLE_COUNT)) {
		fi->skip_count[SKIP_LOG_FILE]++;
		trace_hmfs_skip_log_writeback(inode->i_ino);
		goto skip_write;
	}
	fi->skip_count[SKIP_LOG_FILE] = 0;

	/* skip writing during file defragment */
	if (is_inode_flag_set(inode, FI_DO_DEFRAG))
		goto skip_write;

	trace_hmfs_writepages(mapping->host, wbc, DATA);

	/* to avoid spliting IOs due to mixed WB_SYNC_ALL and WB_SYNC_NONE */
	if (wbc->sync_mode == WB_SYNC_ALL)
		atomic_inc(&sbi->wb_sync_req[DATA]);
	else if (atomic_read(&sbi->wb_sync_req[DATA]))
		goto skip_write;

#ifdef CONFIG_BLK_DEV_THROTTLING
	if (!(current->flags & PF_MUTEX_GC)) {
		current->wb_stat.bios = 0;
		blk_throtl_get_quota(inode->i_sb->s_bdev, PAGE_SIZE,
				     msecs_to_jiffies(100), true);
	}
#endif

	if (__should_serialize_io(inode, wbc)) {
		mutex_lock(&sbi->writepages);
		locked = true;
	}

	if (locked && __is_need_pu_align(inode, wbc)) {
		int dirty_pages = get_dirty_pages(inode);
		int offset = ((NEXT_FREE_BLKADDR(sbi,
					CURSEG_I(sbi, CURSEG_COLD_DATA)) -
					MAIN_BLKADDR(sbi)) % sbi->pu_size[SLC_MODE]);

		if (dirty_pages > wbc->nr_to_write)
			dirty_pages = wbc->nr_to_write;

		dirty_pages -= ((dirty_pages + offset) % sbi->pu_size[SLC_MODE]);
		if (dirty_pages <= 0) {
			fi->skip_count[SKIP_PU_ALIGN]++;

			mutex_unlock(&sbi->writepages);
			goto skip_write;
		}

		wbc->nr_to_write = dirty_pages;
	}
	fi->skip_count[SKIP_PU_ALIGN] = 0;

	blk_start_plug(&plug);
	ret = f2fs_write_cache_pages(mapping, wbc, io_type);
	blk_finish_plug(&plug);

	if (locked)
		mutex_unlock(&sbi->writepages);

#ifdef CONFIG_BLK_DEV_THROTTLING
	if (!(current->flags & PF_MUTEX_GC)) {
		if (current->wb_stat.bios)
			blk_throtl_get_quotas(inode->i_sb->s_bdev,
					     PAGE_SIZE,
					     msecs_to_jiffies(100),
					     true,
					     current->wb_stat.bios - 1);
	}
#endif

	if (wbc->sync_mode == WB_SYNC_ALL)
		atomic_dec(&sbi->wb_sync_req[DATA]);
	/*
	 * if some pages were truncated, we cannot guarantee its mapping->host
	 * to detect pending bios.
	 */

	hmfs_remove_dirty_inode(inode);
	return ret;

skip_write:
	wbc->pages_skipped += get_dirty_pages(inode);
	trace_hmfs_writepages(mapping->host, wbc, DATA);
	return 0;
}

static int f2fs_write_data_pages(struct address_space *mapping,
			    struct writeback_control *wbc)
{
	struct inode *inode = mapping->host;

	return __f2fs_write_data_pages(mapping, wbc,
			F2FS_I(inode)->cp_task == current ?
			FS_CP_DATA_IO : FS_DATA_IO);
}

static void f2fs_write_failed(struct address_space *mapping, loff_t to)
{
	struct inode *inode = mapping->host;
	loff_t i_size = i_size_read(inode);

	if (to > i_size) {
		down_write(&F2FS_I(inode)->i_gc_rwsem[WRITE]);
		down_write(&F2FS_I(inode)->i_mmap_sem);

		truncate_pagecache(inode, i_size);
		hmfs_truncate_blocks(inode, i_size, true, true);

		up_write(&F2FS_I(inode)->i_mmap_sem);
		up_write(&F2FS_I(inode)->i_gc_rwsem[WRITE]);
	}
}

static int prepare_write_begin(struct f2fs_sb_info *sbi,
			struct page *page, loff_t pos, unsigned len,
			block_t *blk_addr, bool *node_changed)
{
	struct inode *inode = page->mapping->host;
	pgoff_t index = page->index;
	struct dnode_of_data dn;
	struct page *ipage;
	bool locked = false;
	struct extent_info ei = {0,0,0};
	int err = 0;
	int flag;
	bool lock2 = false;

	/*
	 * we already allocated all the blocks, so we don't need to get
	 * the block addresses when there is no need to fill the page.
	 */
	if (!f2fs_has_inline_data(inode) && len == PAGE_SIZE &&
			!is_inode_flag_set(inode, FI_NO_PREALLOC))
		return 0;

	/* f2fs_lock_op avoids race between write CP and convert_inline_page */
	if (f2fs_has_inline_data(inode) && pos + (loff_t)len > (loff_t)MAX_INLINE_DATA(inode))
		flag = F2FS_GET_BLOCK_DEFAULT;
	else
		flag = F2FS_GET_BLOCK_PRE_AIO;

	if (f2fs_has_inline_data(inode) ||
			(pos & PAGE_MASK) >= i_size_read(inode)) {
		lock2 = __hmfs_do_map_lock(sbi, flag, true, false);
		locked = true;
	}
restart:
	/* check inline_data */
	ipage = hmfs_get_node_page(sbi, inode->i_ino);
	if (IS_ERR(ipage)) {
		err = PTR_ERR(ipage);
		goto unlock_out;
	}

	set_new_dnode(&dn, inode, ipage, ipage, 0);

	if (f2fs_has_inline_data(inode)) {
		if (pos + len <= MAX_INLINE_DATA(inode)) {
			hmfs_do_read_inline_data(page, ipage);
			set_inode_flag(inode, FI_DATA_EXIST);
			if (inode->i_nlink)
				set_inline_node(ipage);
		} else {
			err = hmfs_convert_inline_page(&dn, page);
			if (err)
				goto out;
			if (dn.data_blkaddr == NULL_ADDR)
				err = hmfs_get_block(&dn, index);
		}
	} else if (locked) {
		err = hmfs_get_block(&dn, index);
	} else {
		if (hmfs_lookup_extent_cache(inode, index, &ei)) {
			dn.data_blkaddr = ei.blk + index - ei.fofs;
		} else {
			/* hole case */
			err = hmfs_get_dnode_of_data(&dn, index, LOOKUP_NODE);
			if (err || dn.data_blkaddr == NULL_ADDR) {
				f2fs_put_dnode(&dn);
				lock2 = __hmfs_do_map_lock(sbi, F2FS_GET_BLOCK_PRE_AIO,
					      true, false);
				WARN_ON(flag != F2FS_GET_BLOCK_PRE_AIO);
				locked = true;
				goto restart;
			}
		}
	}

	/* convert_inline_page can make node_changed */
	*blk_addr = dn.data_blkaddr;
	*node_changed = dn.node_changed;
out:
	f2fs_put_dnode(&dn);
unlock_out:
	if (locked)
		(void)__hmfs_do_map_lock(sbi, flag, false, lock2);
	return err;
}

/*
 * __hmfs_io_throttle - restrict io based on system load
 *
 * @throttle_base_times: modulus from free sections
 * @is_tail: whether or not there are enough free space
 * @restrict_hard: set to true if current task belongs to BG
 * @length: the restricted io's size (bytes)
 */
static void __hmfs_io_throttle(struct f2fs_sb_info *sbi,
					 unsigned int throttle_base_time, bool is_tail,
					 bool restrict_hard,
					 unsigned int length)
{
	unsigned int len = length;
	unsigned int interval = throttle_base_time;
	unsigned int io_throttle_time = sbi->io_throttle_time;
	unsigned int io_throttle_time_max = sbi->io_throttle_time_max;
	struct timespec64 tu;
	static atomic_t hard_accumulated, slight_accumulated;
	bool sleep = false;
	int accumulate;

	if (is_tail)
		interval = interval << 2;

	if (restrict_hard)
		interval = interval << 3;

	if (has_not_enough_free_secs(sbi, 0, 0))
		interval = interval << 2;

	/* transfer bytes to PAGE_SIZE */
	len = len >> 12;
	interval = len * interval;

	/*
	 * Sleep too often when small IO are issured is not a good idea,
	 * thus only sleep when accumulated time reach threshold.
	 */
	if (restrict_hard) {
		accumulate = atomic_add_return(interval, &hard_accumulated);

		if (accumulate > io_throttle_time) {
			atomic_set(&hard_accumulated, 0);
			sleep = true;
		}
	} else {
		accumulate = atomic_add_return(interval, &slight_accumulated);

		if (accumulate > io_throttle_time) {
			atomic_set(&slight_accumulated, 0);
			sleep = true;
		}
	}

	if (sleep) {
		io_throttle_time = accumulate;

		if (io_throttle_time > io_throttle_time_max)
			io_throttle_time = io_throttle_time_max;

		tu.tv_sec = 0;
		tu.tv_nsec = io_throttle_time * US_TO_NS;

		if (!timespec64_valid(&tu))
			return;

		current->restart_block.nanosleep.type = TT_NONE;
		current->restart_block.nanosleep.rmtp = NULL;

		hrtimer_nanosleep(&tu, HRTIMER_MODE_REL, CLOCK_MONOTONIC);

		bd_mutex_lock(&sbi->bd_mutex);
		if (restrict_hard) {
			inc_bd_val(sbi, throttle_cnt_bg, 1);
			inc_bd_val(sbi, throttle_time_bg, io_throttle_time);
			max_bd_val(sbi, max_throttle_time_bg, io_throttle_time);
		} else {
			inc_bd_val(sbi, throttle_cnt_fg, 1);
			inc_bd_val(sbi, throttle_time_fg, io_throttle_time);
			max_bd_val(sbi, max_throttle_time_fg, io_throttle_time);
		}
		bd_mutex_unlock(&sbi->bd_mutex);
	}
}

static unsigned int hmfs_get_throttle_policy(struct f2fs_sb_info *sbi,
					 unsigned int free_secs,  int *level_ret)
{
	unsigned int throttle_base_time = 0;
	int interval;
	int *waterline = sbi->gc_stat.level;

	interval = waterline[BG_GC_LEVEL4] - waterline[BG_GC_LEVEL5];

	if (free_secs > waterline[BG_GC_LEVEL1]) {
		*level_ret = BG_GC_LEVEL1;
	} else if (free_secs > waterline[BG_GC_LEVEL2]) {
		*level_ret = BG_GC_LEVEL2;
		throttle_base_time = THROTTLE_LEVEL2;
	} else if (free_secs > waterline[BG_GC_LEVEL3]) {
		*level_ret = BG_GC_LEVEL3;
		throttle_base_time = THROTTLE_LEVEL3;
	} else if (free_secs > waterline[BG_GC_LEVEL4]) {
		*level_ret = BG_GC_LEVEL4;
		throttle_base_time = THROTTLE_LEVEL4;
	} else if (free_secs > waterline[BG_GC_LEVEL5]) {
		*level_ret = BG_GC_LEVEL5;
		throttle_base_time = THROTTLE_LEVEL5 + (interval -
				(free_secs - waterline[BG_GC_LEVEL5])) / 2;
		if (throttle_base_time > THROTTLE_LEVEL6)
			throttle_base_time = THROTTLE_LEVEL6;
	} else {
		*level_ret = BG_GC_LEVEL6;
		throttle_base_time = THROTTLE_LEVEL6;
	}

	return throttle_base_time;
}

/*
 * Note: We will sleep a while to reduce max latency at end of capacity,
 * so we must call this function out of the lock.
 */
void hmfs_io_throttle(struct f2fs_sb_info *sbi, unsigned int len)
{
	struct blkcg *blkcg = NULL;
	unsigned int free_secs;
	unsigned int throttle_base_time;
	block_t free_block_count;
	int level_type = 0;
	bool istail = false;
	bool restrict_hard = false;

	if (sbi->io_throttle_time == 0 ||
				sbi->io_throttle_time_max == 0)
		return;

	if (current->flags & (PF_MEMALLOC | PF_SWAPWRITE |
				PF_KSWAPD | PF_MUTEX_GC))
		return;

	rcu_read_lock();
	blkcg = task_blkcg(current);
	if (!blkcg) {
		rcu_read_unlock();
		return;
	}

	/* restrict harder if current task is BG */
	if (blkcg->type == BLK_THROTL_BG)
		restrict_hard = true;
	rcu_read_unlock();

	/*
	 * the calculation of istail and level_type is referenced from
	 * gc_thread_func().
	 */
	free_secs = free_sections(sbi);
	free_block_count = hmfs_get_free_block_count(sbi);
	if (hmfs_disk_is_frag(sbi, free_block_count, free_secs))
		istail = true;

	throttle_base_time = hmfs_get_throttle_policy(sbi, free_secs, &level_type);

	if (restrict_hard) {
		/*
		 * if current task is BG, skip if waterline is
		 * level1.
		 */
		if (level_type == BG_GC_LEVEL1)
			return;

	} else {
		/*
		 * if current task is not BG, skip if waterline is
		 * less than level6.
		 */
		if (level_type < BG_GC_LEVEL6)
			return;

		throttle_base_time = throttle_base_time - THROTTLE_LEVEL5;
	}

	__hmfs_io_throttle(sbi, throttle_base_time, istail, restrict_hard, len);
}

static int f2fs_write_begin(struct file *file, struct address_space *mapping,
		loff_t pos, unsigned len, unsigned flags,
		struct page **pagep, void **fsdata)
{
	struct inode *inode = mapping->host;
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct page *page = NULL;
	pgoff_t index = ((unsigned long long) pos) >> PAGE_SHIFT;
	bool need_balance = false, drop_atomic = false;
	block_t blkaddr = NULL_ADDR;
	int err = 0;

	if (trace_android_fs_datawrite_start_enabled()) {
		char *path, pathbuf[MAX_TRACE_PATHBUF_LEN];

		path = android_fstrace_get_pathname(pathbuf,
						    MAX_TRACE_PATHBUF_LEN,
						    inode);
		trace_android_fs_datawrite_start(inode, pos, len,
						 current->pid, path,
						 current->comm);
	}
	trace_hmfs_write_begin(inode, pos, len, flags);

	hmfs_io_throttle(sbi, PAGE_SIZE);

	err = f2fs_is_checkpoint_ready(sbi);
	if (err)
		goto fail;

	if ((f2fs_is_atomic_file(inode) &&
			!hmfs_available_free_memory(sbi, INMEM_PAGES)) ||
			is_inode_flag_set(inode, FI_ATOMIC_REVOKE_REQUEST)) {
		err = -ENOMEM;
		drop_atomic = true;
		goto fail;
	}

	/*
	 * We should check this at this moment to avoid deadlock on inode page
	 * and #0 page. The locking rule for inline_data conversion should be:
	 * lock_page(page #0) -> lock_page(inode_page)
	 */
	if (index != 0) {
		err = hmfs_convert_inline_inode(inode);
		if (err)
			goto fail;
	}
repeat:
	/*
	 * Do not use grab_cache_page_write_begin() to avoid deadlock due to
	 * wait_for_stable_page. Will wait that below with our IO control.
	 */
	page = f2fs_pagecache_get_page(mapping, index,
				FGP_LOCK | FGP_WRITE | FGP_CREAT, GFP_NOFS);
	if (!page) {
		err = -ENOMEM;
		goto fail;
	}

	*pagep = page;

	err = prepare_write_begin(sbi, page, pos, len,
					&blkaddr, &need_balance);
	if (err)
		goto fail;

	if (need_balance && !IS_NOQUOTA(inode) &&
			has_not_enough_free_secs(sbi, 0, 0)) {
		unlock_page(page);
		hmfs_balance_fs(sbi, true);
		lock_page(page);
		if (page->mapping != mapping) {
			/* The page got truncated from under us */
			f2fs_put_page(page, 1);
			goto repeat;
		}
	}

	hmfs_wait_on_page_writeback(page, DATA, false);

	if (len == PAGE_SIZE || PageUptodate(page))
		return 0;

	if (!(pos & (PAGE_SIZE - 1)) && (pos + len) >= i_size_read(inode)) {
		zero_user_segment(page, len, PAGE_SIZE);
		return 0;
	}

	if (blkaddr == NEW_ADDR) {
		zero_user_segment(page, 0, PAGE_SIZE);
		SetPageUptodate(page);
	} else {
		err = f2fs_submit_page_read(inode, page, blkaddr);
		if (err)
			goto fail;

		lock_page(page);
		if (unlikely(page->mapping != mapping)) {
			f2fs_put_page(page, 1);
			goto repeat;
		}
		if (unlikely(!PageUptodate(page))) {
			err = -EIO;
			goto fail;
		}
	}
	return 0;

fail:
	f2fs_put_page(page, 1);
	f2fs_write_failed(mapping, pos + len);
	if (drop_atomic)
		hmfs_drop_inmem_pages_all(sbi, false);
	return err;
}

static int f2fs_write_end(struct file *file,
			struct address_space *mapping,
			loff_t pos, unsigned len, unsigned copied,
			struct page *page, void *fsdata)
{
	struct inode *inode = page->mapping->host;

	trace_android_fs_datawrite_end(inode, pos, len);
	trace_hmfs_write_end(inode, pos, len, copied);

	/*
	 * This should be come from len == PAGE_SIZE, and we expect copied
	 * should be PAGE_SIZE. Otherwise, we treat it with zero copied and
	 * let generic_perform_write() try to copy data again through copied=0.
	 */
	if (!PageUptodate(page)) {
		if (unlikely(copied != len))
			copied = 0;
		else
			SetPageUptodate(page);
	}
	if (!copied)
		goto unlock_out;

	set_page_dirty(page);

	if (pos + copied > i_size_read(inode))
		f2fs_i_size_write(inode, pos + copied);
unlock_out:
	f2fs_put_page(page, 1);
	f2fs_update_time(F2FS_I_SB(inode), REQ_TIME);
	return copied;
}

static int check_direct_IO(struct inode *inode, struct iov_iter *iter,
			   loff_t offset)
{
	unsigned i_blkbits = READ_ONCE(inode->i_blkbits);
	unsigned blkbits = i_blkbits;
	unsigned blocksize_mask = (1 << blkbits) - 1;
	unsigned long align = offset | iov_iter_alignment(iter);
	struct block_device *bdev = inode->i_sb->s_bdev;

	if (align & blocksize_mask) {
		if (bdev)
			blkbits = blksize_bits(bdev_logical_block_size(bdev));
		blocksize_mask = (1 << blkbits) - 1;
		if (align & blocksize_mask)
			return -EINVAL;
		return 1;
	}
	return 0;
}

#ifdef CONFIG_FS_ENCRYPTION
struct f2fs_crypt_dio {
	struct inode		*inode;
	struct fscrypt_ctx	*ctx;
	void			*private;
	bio_end_io_t		*orig_end_io;
	loff_t			foff;
};

static inline void __f2fs_crypt_end_dio(struct f2fs_crypt_dio *dio,
					struct bio *bio)
{
	bio->bi_private = dio->private;
	bio->bi_end_io = dio->orig_end_io;

	kfree(dio);

	bio_endio(bio);
}

/*
 * Note: We just support the f2fs whose block size is equal to PAGE SIZE now.
 *       This is because current fs/crypt module only can run well
 *       on this case. In the future(Maybe v4.9 or later), we need change
 *       this function to encrypt the data by the block address.
 */
static void f2fs_complete_dio_read(struct work_struct *work)
{
	/*lint -save -e826*/
	struct fscrypt_ctx *ctx =
		container_of(work, struct fscrypt_ctx, r.work);
	/*lint -restore*/
	struct bio *bio = ctx->r.bio;
	struct f2fs_crypt_dio *dio = bio->bi_private;
	struct inode *inode = dio->inode;
	struct bio_vec *bv;
	struct page *page;
	pgoff_t index;
	int i;
	int ret;

	/*lint -save -e704*/
	index = dio->foff >> PAGE_SHIFT;
	/*lint -restore*/

	bio_for_each_segment_all(bv, bio, i) {
		page = bv->bv_page;

		ret = fscrypt_decrypt_dio_page(inode, page, PAGE_SIZE, 0, index);
		if (ret) {
			bio->bi_status = BLK_STS_IOERR;
			break;
		}
		index++;
	}

	fscrypt_release_ctx(ctx);
	__f2fs_crypt_end_dio(dio, bio);
}

static void f2fs_crypt_direct_read_end_io(struct bio *bio)
{
	struct f2fs_crypt_dio *dio = bio->bi_private;
	struct fscrypt_ctx *ctx = dio->ctx;

	if (unlikely(bio->bi_status)) {
		fscrypt_release_ctx(ctx);
		__f2fs_crypt_end_dio(dio, bio);
		return;
	}

	fscrypt_decrypt_dio_bio_pages(ctx, bio, f2fs_complete_dio_read);
}

static void f2fs_crypt_direct_write_end_io(struct bio *bio)
{
	struct f2fs_crypt_dio *dio = bio->bi_private;
	struct bio_vec *bvec;
	int i;

	bio_for_each_segment_all(bvec, bio, i) {
		struct page *page = bvec->bv_page;

		fscrypt_pullback_bio_page(&page, true);

		bvec->bv_page = page;
	}

	/*
	 * Direct IO is only used for user data, IO errors which occur during
	 * the user data IO would not break the file system, so we needn't
	 * check the IO error here to decide if we need stop check point
	 * process or not.
	 */

	__f2fs_crypt_end_dio(dio, bio);
}

/*
 * Note: We just support the f2fs whose block size is equal to PAGE SIZE now.
 *       This is because current fs/crypt module only can run well
 *       on this case. In the future(Maybe v4.9 or later), we need change
 *       this function to encrypt the data by the block address.
 */
static int f2fs_encrypt_dio_pages(struct inode *inode, struct bio *bio,
				  loff_t offset)
{
	struct page *page;
	struct page *encrypted_page;
	struct bio_vec *bvec;
	pgoff_t index;
	int max;
	int i;

	/*lint -save -e704*/
	index = offset >> PAGE_SHIFT;
	/*lint -restore*/

	bio_for_each_segment_all(bvec, bio, i) {
		page = bvec->bv_page;

		encrypted_page = fscrypt_encrypt_dio_page(inode, page,
					PAGE_SIZE, 0, index, GFP_NOFS);
		if (IS_ERR(encrypted_page))
			goto error;

		bvec->bv_page = encrypted_page;
		index++;
	}

	return 0;

error:
	max = i;
	bio_for_each_segment_all(bvec, bio, i) {
		if (i >= max)
			break;

		page = bvec->bv_page;

		fscrypt_pullback_bio_page(&page, true);

		bvec->bv_page = page;
	}
	/*lint -save -e712*/
	return PTR_ERR(encrypted_page);
	/*lint -restore*/
}

static void __set_unistore_bio_param(struct bio *bio, struct inode *inode,
		loff_t foff)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	int rw_hint = hmfs_rw_hint_to_seg_type(sbi, inode->i_write_hint);
	bool write = (bio_op(bio) == REQ_OP_WRITE);

	if (!write)
		return;

	switch (rw_hint) {
		case CURSEG_HOT_DATA:
			bio->mas_bio.stream_type = STREAM_HOT_DATA;
			break;
		case CURSEG_COLD_DATA:
			bio->mas_bio.stream_type = STREAM_COLD_DATA;
			break;
		default:
			f2fs_bug_on(sbi, 1);
			break;
	}

	if (hmfs_is_oob_enable(sbi)) {
		bio->mas_bio.data_ino = inode->i_ino;
		/* DIO: no atomic, recovery all oob */
		bio->mas_bio.data_idx = (F2FS_BYTES_TO_BLK(foff) | OOB_FSYNC_BIT);
		hmfs_file_check_switch_stream(inode, bio->mas_bio.stream_type);
	}

	hmfs_bio_set_flash_mode(sbi, bio, SECTOR_TO_BLOCK(bio->bi_iter.bi_sector),
			bio->mas_bio.stream_type);
}

static void f2fs_submit_direct(struct bio *bio, struct inode *inode,
			       loff_t foff)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct f2fs_crypt_dio *dio;
	struct fscrypt_ctx *ctx;
	/*lint -save -e737*/
	bool write = (bio_op(bio) == REQ_OP_WRITE);
	/*lint -restore*/
	int ret;

	dio = kzalloc(sizeof(struct f2fs_crypt_dio), GFP_NOFS);
	if (!dio) {
		ret = -ENOMEM;
		goto dio_err;
	}

	dio->inode = inode;
	dio->private = bio->bi_private;
	dio->foff = foff;
	dio->orig_end_io = bio->bi_end_io;

	if (!write) {
		/*lint -save -e712*/
		ctx = fscrypt_get_ctx(inode, GFP_NOFS);
		if (IS_ERR(ctx)) {
			ret = PTR_ERR(ctx);
			goto ctx_err;
		}
		/*lint -restore*/

		dio->ctx = ctx;

		bio->bi_end_io = f2fs_crypt_direct_read_end_io;
	} else {
		ret = f2fs_encrypt_dio_pages(inode, bio, foff);
		if (ret)
			goto ctx_err;
		bio->bi_end_io = f2fs_crypt_direct_write_end_io;
	}

	bio->bi_private = dio;

	current->wb_stat.bios++;

	if (write) {
		__set_unistore_bio_param(bio, inode, foff);
		__hmfs_submit_write_bio(sbi, bio, DATA);
	} else {
		submit_bio(bio);
	}

	return;

ctx_err:
	kfree(dio);
dio_err:
	if (ret != 0)
		bio->bi_status = BLK_STS_IOERR;
	bio_endio(bio);
}

static void hmfs_submit_direct2(struct bio *bio, struct inode *inode,
			       loff_t foff)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	bool write = (bio_op(bio) == REQ_OP_WRITE);

	__set_unistore_bio_param(bio, inode, foff);

	hmfs_submit_merged_write(sbi, DATA);

	current->wb_stat.bios++;

	if (write) {
		__hmfs_submit_write_bio(sbi, bio, DATA);
	} else {
		submit_bio(bio);
	}

	return;
}
#endif

static ssize_t f2fs_direct_IO(struct kiocb *iocb, struct iov_iter *iter)
{
	struct address_space *mapping = iocb->ki_filp->f_mapping;
	struct inode *inode = mapping->host;
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct f2fs_inode_info *fi = F2FS_I(inode);
	size_t count = iov_iter_count(iter);
	loff_t offset = iocb->ki_pos;
	int rw = iov_iter_rw(iter);
	int err;
	enum rw_hint hint = iocb->ki_hint;
	int whint_mode = F2FS_OPTION(sbi).whint_mode;
	bool do_opu = false;
	bool is_aio = !is_sync_kiocb(iocb);

	err = check_direct_IO(inode, iter, offset);
	if (err)
		return err < 0 ? err : 0;

	if (f2fs_force_buffered_io(inode, iocb, iter))
		return 0;

	do_opu = allow_outplace_dio(inode, iocb, iter);

	trace_hmfs_direct_IO_enter(inode, offset, count, rw);

	if (rw == WRITE)
		hmfs_io_throttle(sbi, count);

	if (trace_android_fs_dataread_start_enabled() &&
	    (rw == READ)) {
		char *path, pathbuf[MAX_TRACE_PATHBUF_LEN];

		path = android_fstrace_get_pathname(pathbuf,
						    MAX_TRACE_PATHBUF_LEN,
						    inode);
		trace_android_fs_dataread_start(inode, offset,
						count, current->pid, path,
						current->comm);
	}
	if (trace_android_fs_datawrite_start_enabled() &&
	    (rw == WRITE)) {
		char *path, pathbuf[MAX_TRACE_PATHBUF_LEN];

		path = android_fstrace_get_pathname(pathbuf,
						    MAX_TRACE_PATHBUF_LEN,
						    inode);
		trace_android_fs_datawrite_start(inode, offset, count,
						 current->pid, path,
						 current->comm);
	}
	if (rw == WRITE && whint_mode == WHINT_MODE_OFF)
		iocb->ki_hint = WRITE_LIFE_NOT_SET;

	current->wb_stat.bios = 0;
	blk_throtl_get_quota(inode->i_sb->s_bdev, PAGE_SIZE,
			msecs_to_jiffies(100), true);

	if (iocb->ki_flags & IOCB_NOWAIT) {
		if (!down_read_trylock(&fi->i_gc_rwsem[rw])) {
			iocb->ki_hint = hint;
			err = -EAGAIN;
			goto out;
		}
		if (do_opu && !down_read_trylock(&fi->i_gc_rwsem[READ])) {
			up_read(&fi->i_gc_rwsem[rw]);
			iocb->ki_hint = hint;
			err = -EAGAIN;
			goto out;
		}
	} else {
		down_read(&fi->i_gc_rwsem[rw]);
		if (do_opu)
			down_read(&fi->i_gc_rwsem[READ]);
	}

#ifdef CONFIG_FS_ENCRYPTION
	if (f2fs_encrypted_inode(inode) && S_ISREG(inode->i_mode) &&
		!f2fs_inline_encrypted_inode(inode)) {
		err = __blockdev_direct_IO(iocb, inode, inode->i_sb->s_bdev,
					   iter, get_data_block_dio,
					   NULL, f2fs_submit_direct,
					   DIO_LOCKING | DIO_SKIP_HOLES);
	} else {
		/*lint -save -e712*/
		err = __blockdev_direct_IO(iocb, inode, inode->i_sb->s_bdev,
					   iter, get_data_block_dio,
					   NULL, hmfs_submit_direct2,
					   DIO_LOCKING | DIO_SKIP_HOLES);

		/*lint -restore*/
	}
#else
	err = blockdev_direct_IO(iocb, inode, iter, get_data_block_dio);
#endif

	if (do_opu)
		up_read(&fi->i_gc_rwsem[READ]);

	up_read(&fi->i_gc_rwsem[rw]);

	if (do_opu)
		hmfs_balance_fs(sbi, true);

	if (rw == WRITE) {
		/* aio may free iocb after blockdev_direct_IO */
		if (whint_mode == WHINT_MODE_OFF && !is_aio)
			iocb->ki_hint = hint;
		if (err > 0) {
			f2fs_update_iostat(F2FS_I_SB(inode), APP_DIRECT_IO,
									err);
			if (!do_opu)
				set_inode_flag(inode, FI_UPDATE_WRITE);
		} else if (err < 0) {
			f2fs_write_failed(mapping, offset + count);
		}
	}
out:
	if (current->wb_stat.bios)
		blk_throtl_get_quotas(inode->i_sb->s_bdev,
				PAGE_SIZE,
				msecs_to_jiffies(100),
				true,
				current->wb_stat.bios - 1);

	if (trace_android_fs_dataread_start_enabled() &&
	    (rw == READ))
		trace_android_fs_dataread_end(inode, offset, count);
	if (trace_android_fs_datawrite_start_enabled() &&
	    (rw == WRITE))
		trace_android_fs_datawrite_end(inode, offset, count);

	trace_hmfs_direct_IO_exit(inode, offset, count, rw, err);

	return err;
}

void hmfs_invalidate_page(struct page *page, unsigned int offset,
							unsigned int length)
{
	struct inode *inode = page->mapping->host;
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);

	if (inode->i_ino >= F2FS_ROOT_INO(sbi) &&
		(offset % PAGE_SIZE || length != PAGE_SIZE))
		return;

	if (PageDirty(page)) {
		if (inode->i_ino == F2FS_META_INO(sbi)) {
			dec_page_count(sbi, F2FS_DIRTY_META);
		} else if (inode->i_ino == F2FS_NODE_INO(sbi)) {
			dec_page_count(sbi, F2FS_DIRTY_NODES);
		} else {
			inode_dec_dirty_pages(inode);
			hmfs_remove_dirty_inode(inode);
		}
	}

	clear_cold_data(page);

	/* This is atomic written page, keep Private */
	if (IS_ATOMIC_WRITTEN_PAGE(page))
		return hmfs_drop_inmem_page(inode, page);

	f2fs_clear_page_private(page);
	clear_cold_data(page);
}

int hmfs_release_page(struct page *page, gfp_t wait)
{
	/* If this is dirty page, keep PagePrivate */
	if (PageDirty(page))
		return 0;

	/* This is atomic written page, keep Private */
	if (IS_ATOMIC_WRITTEN_PAGE(page))
		return 0;

	clear_cold_data(page);
	f2fs_clear_page_private(page);
	return 1;
}

static int f2fs_set_data_page_dirty(struct page *page)
{
	struct address_space *mapping = page->mapping;
	struct inode *inode = mapping->host;

	trace_hmfs_set_page_dirty(page, DATA);

	if (!PageUptodate(page))
		SetPageUptodate(page);

	if (f2fs_is_atomic_file(inode) && !f2fs_is_commit_atomic_write(inode)) {
		if (!IS_ATOMIC_WRITTEN_PAGE(page)) {
			hmfs_register_inmem_page(inode, page);
			return 1;
		}
		/*
		 * Previously, this page has been registered, we just
		 * return here.
		 */
		return 0;
	}

	if (!PageDirty(page)) {
		__set_page_dirty_nobuffers(page);
		hmfs_update_dirty_page(inode, page);
		return 1;
	}
	return 0;
}

static sector_t f2fs_bmap(struct address_space *mapping, sector_t block)
{
	struct inode *inode = mapping->host;

	if (f2fs_has_inline_data(inode))
		return 0;

	/* make sure allocating whole blocks */
	if (mapping_tagged(mapping, PAGECACHE_TAG_DIRTY))
		filemap_write_and_wait(mapping);

	return generic_block_bmap(mapping, block, get_data_block_bmap);
}

#ifdef CONFIG_MIGRATION
#include <linux/migrate.h>

int hmfs_migrate_page(struct address_space *mapping,
		struct page *newpage, struct page *page, enum migrate_mode mode)
{
	int rc, extra_count;
	struct f2fs_inode_info *fi = F2FS_I(mapping->host);
	bool atomic_written = IS_ATOMIC_WRITTEN_PAGE(page);

	BUG_ON(PageWriteback(page));

	/* migrating an atomic written page is safe with the inmem_lock hold */
	if (atomic_written) {
		if (mode != MIGRATE_SYNC)
			return -EBUSY;
		if (!mutex_trylock(&fi->inmem_lock))
			return -EAGAIN;
	}

	/* one extra reference was held for atomic_write page */
	extra_count = atomic_written ? 1 : 0;
	rc = migrate_page_move_mapping(mapping, newpage,
				page, NULL, mode, extra_count);
	if (rc != MIGRATEPAGE_SUCCESS) {
		if (atomic_written)
			mutex_unlock(&fi->inmem_lock);
		return rc;
	}

	if (atomic_written) {
		struct inmem_pages *cur;
		list_for_each_entry(cur, &fi->inmem_pages, list)
			if (cur->page == page) {
				cur->page = newpage;
				break;
			}
		mutex_unlock(&fi->inmem_lock);
		put_page(page);
		get_page(newpage);
	}

	if (PagePrivate(page)) {
		f2fs_set_page_private(newpage, page_private(page));
		f2fs_clear_page_private(page);
	}

	if (mode != MIGRATE_SYNC_NO_COPY)
		migrate_page_copy(newpage, page);
	else
		migrate_page_states(newpage, page);

	return MIGRATEPAGE_SUCCESS;
}
#endif

const struct address_space_operations hmfs_dblock_aops = {
	.readpage	= f2fs_read_data_page,
	.readpages	= f2fs_read_data_pages,
	.writepage	= f2fs_write_data_page,
	.writepages	= f2fs_write_data_pages,
	.write_begin	= f2fs_write_begin,
	.write_end	= f2fs_write_end,
	.set_page_dirty	= f2fs_set_data_page_dirty,
	.invalidatepage	= hmfs_invalidate_page,
	.releasepage	= hmfs_release_page,
	.direct_IO	= f2fs_direct_IO,
	.bmap		= f2fs_bmap,
#ifdef CONFIG_MIGRATION
	.migratepage	= hmfs_migrate_page,
#endif
};

void hmfs_clear_radix_tree_dirty_tag(struct page *page)
{
	struct address_space *mapping = page_mapping(page);
	unsigned long flags;

	if (!mapping)
		return;
	spin_lock_irqsave(&mapping->tree_lock, flags);
	radix_tree_tag_clear(&mapping->page_tree, page_index(page),
					PAGECACHE_TAG_DIRTY);
	spin_unlock_irqrestore(&mapping->tree_lock, flags);
}

int __init hmfs_init_post_read_processing(void)
{
	bio_post_read_ctx_cache = KMEM_CACHE(bio_post_read_ctx, 0);
	if (!bio_post_read_ctx_cache)
		goto fail;
	bio_post_read_ctx_pool =
		mempool_create_slab_pool(NUM_PREALLOC_POST_READ_CTXS,
					 bio_post_read_ctx_cache);
	if (!bio_post_read_ctx_pool)
		goto fail_free_cache;
	return 0;

fail_free_cache:
	kmem_cache_destroy(bio_post_read_ctx_cache);
fail:
	return -ENOMEM;
}

void __exit hmfs_destroy_post_read_processing(void)
{
	mempool_destroy(bio_post_read_ctx_pool);
	kmem_cache_destroy(bio_post_read_ctx_cache);
}
