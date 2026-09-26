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
 * Create: 2020-5-20
 *
 */

#define pr_fmt(fmt) "[HYPERHOLD]" fmt

#include <linux/kernel.h>
#include <linux/swap.h>

#include "zram_drv.h"
#include "hyperhold.h"
#include "hyperhold_internal.h"

#include "hyperhold_list.h"
#include "hyperhold_area.h"
#include "hyperhold_lru_rmap.h"

#define esentry_extid(e) ((e) >> EXTENT_SHIFT)

void zram_set_memcg(struct zram *zram, u32 index, struct mem_cgroup *mcg)
{
	unsigned short mcg_id = mcg ? mcg->id.id : 0;

	hh_list_set_mcgid(obj_idx(zram->area, index),
				zram->area->obj_table, mcg_id);
}

struct mem_cgroup *zram_get_memcg(struct zram *zram, u32 index)
{
	unsigned short mcg_id;

	mcg_id = hh_list_get_mcgid(obj_idx(zram->area, index),
				zram->area->obj_table);

	return get_mem_cgroup(mcg_id);
}

int zram_get_memcg_coldest_index(struct hyperhold_area *area,
				 struct mem_cgroup *mcg,
				 int *index, int max_cnt)
{
	int cnt = 0;
	u32 i;

	if (!area) {
		hh_print(HHLOG_ERR, "NULL area\n");
		return 0;
	}
	if (!area->obj_table) {
		hh_print(HHLOG_ERR, "NULL table\n");
		return 0;
	}
	if (!mcg) {
		hh_print(HHLOG_ERR, "NULL mcg\n");
		return 0;
	}
	if (!index) {
		hh_print(HHLOG_ERR, "NULL index\n");
		return 0;
	}

	hh_lock_list(mcg_idx(area, mcg->id.id), area->obj_table);
	hh_list_for_each_entry_reverse(i,
		mcg_idx(area, mcg->id.id), area->obj_table) { /*lint !e666*/
		if (i >= (u32)area->nr_objs) {
			hh_print(HHLOG_ERR, "index = %d invalid\n", i);
			continue;
		}
		index[cnt++] = i;
		if (cnt >= max_cnt)
			break;
	}
	hh_unlock_list(mcg_idx(area, mcg->id.id), area->obj_table);

	return cnt;
}

int zram_rmap_get_extent_index(struct hyperhold_area *area,
			       int ext_id, int *index)
{
	int cnt = 0;
	u32 i;

	if (!area) {
		hh_print(HHLOG_ERR, "NULL area\n");
		return 0;
	}
	if (!area->obj_table) {
		hh_print(HHLOG_ERR, "NULL table\n");
		return 0;
	}
	if (!index) {
		hh_print(HHLOG_ERR, "NULL index\n");
		return 0;
	}
	if (ext_id < 0 || ext_id >= area->nr_exts) {
		hh_print(HHLOG_ERR, "ext = %d invalid\n", ext_id);
		return 0;
	}

	hh_lock_list(ext_idx(area, ext_id), area->obj_table);
	hh_list_for_each_entry(i, ext_idx(area, ext_id), area->obj_table) { /*lint !e666*/
		/*lint -e548*/
		if (cnt >= (int)EXTENT_MAX_OBJ_CNT) {
			WARN_ON_ONCE(1);
			break;
		}
		/*lint +e548*/
		index[cnt++] = i;
	}
	hh_unlock_list(ext_idx(area, ext_id), area->obj_table);

	return cnt;
}

void zram_lru_add(struct zram *zram, u32 index, struct mem_cgroup *mcg)
{
	unsigned long size;
	struct hyperhold_stat *stat = hyperhold_get_stat_obj();

	if (!stat) {
		hh_print(HHLOG_ERR, "NULL stat\n");
		return;
	}
	if (!zram) {
		hh_print(HHLOG_ERR, "NULL zram\n");
		return;
	}
	if (!mcg || !mcg->zram || !mcg->zram->area) {
		hh_print(HHLOG_ERR, "invalid mcg\n");
		return;
	}
	if (index >= (u32)zram->area->nr_objs) {
		hh_print(HHLOG_ERR, "index = %d invalid\n", index);
		return;
	}
	if (zram_test_flag(zram, index, ZRAM_WB)) {
		hh_print(HHLOG_ERR, "WB object, index = %d\n", index);
		return;
	}
	if (zram_test_flag(zram, index, ZRAM_SAME))
		return;

	zram_set_memcg(zram, index, mcg);
	hh_list_add(obj_idx(zram->area, index),
			mcg_idx(zram->area, mcg->id.id),
			zram->area->obj_table);

	size = zram_get_obj_size(zram, index);

	atomic64_add(size, &mcg->zram_stored_size);
	atomic64_inc(&mcg->zram_page_size);
	atomic64_add(size, &stat->zram_stored_size);
	atomic64_inc(&stat->zram_stored_pages);
}

void zram_lru_add_tail(struct zram *zram, u32 index, struct mem_cgroup *mcg)
{
	unsigned long size;
	struct hyperhold_stat *stat = hyperhold_get_stat_obj();

	if (!stat) {
		hh_print(HHLOG_ERR, "NULL stat\n");
		return;
	}
	if (!zram) {
		hh_print(HHLOG_ERR, "NULL zram\n");
		return;
	}
	if (!mcg || !mcg->zram || !mcg->zram->area) {
		hh_print(HHLOG_ERR, "invalid mcg\n");
		return;
	}
	if (index >= (u32)zram->area->nr_objs) {
		hh_print(HHLOG_ERR, "index = %d invalid\n", index);
		return;
	}
	if (zram_test_flag(zram, index, ZRAM_WB)) {
		hh_print(HHLOG_ERR, "WB object, index = %d\n", index);
		return;
	}
	if (zram_test_flag(zram, index, ZRAM_SAME))
		return;

	zram_set_memcg(zram, index, mcg);
	hh_list_add_tail(obj_idx(zram->area, index),
			mcg_idx(zram->area, mcg->id.id),
			zram->area->obj_table);

	size = zram_get_obj_size(zram, index);

	atomic64_add(size, &mcg->zram_stored_size);
	atomic64_inc(&mcg->zram_page_size);
	atomic64_add(size, &stat->zram_stored_size);
	atomic64_inc(&stat->zram_stored_pages);
}

void zram_lru_del(struct zram *zram, u32 index)
{
	struct mem_cgroup *mcg = NULL;
	unsigned long size;
	struct hyperhold_stat *stat = hyperhold_get_stat_obj();

	if (!stat) {
		hh_print(HHLOG_ERR, "NULL stat\n");
		return;
	}
	if (!zram || !zram->area) {
		hh_print(HHLOG_ERR, "NULL zram\n");
		return;
	}
	if (index >= (u32)zram->area->nr_objs) {
		hh_print(HHLOG_ERR, "index = %d invalid\n", index);
		return;
	}
	if (zram_test_flag(zram, index, ZRAM_WB)) {
		hh_print(HHLOG_ERR, "WB object, index = %d\n", index);
		return;
	}

	mcg = zram_get_memcg(zram, index);
	if (!mcg || !mcg->zram || !mcg->zram->area)
		return;
	if (zram_test_flag(zram, index, ZRAM_SAME))
		return;

	size = zram_get_obj_size(zram, index);
	hh_list_del(obj_idx(zram->area, index),
			mcg_idx(zram->area, mcg->id.id),
			zram->area->obj_table);
	zram_set_memcg(zram, index, NULL);

	atomic64_sub(size, &mcg->zram_stored_size);
	atomic64_dec(&mcg->zram_page_size);
	atomic64_sub(size, &stat->zram_stored_size);
	atomic64_dec(&stat->zram_stored_pages);
}

void zram_rmap_insert(struct zram *zram, u32 index)
{
	unsigned long eswpentry;
	u32 ext_id;

	if (!zram) {
		hh_print(HHLOG_ERR, "NULL zram\n");
		return;
	}
	if (index >= (u32)zram->area->nr_objs) {
		hh_print(HHLOG_ERR, "index = %d invalid\n", index);
		return;
	}
#ifdef CONFIG_ZRAM_DEDUP
	eswpentry = zram_get_direct_handle(zram, index);
#else
	eswpentry = zram_get_handle(zram, index);
#endif
	ext_id = esentry_extid(eswpentry);
	hh_list_add_tail(obj_idx(zram->area, index),
			ext_idx(zram->area, ext_id),
			zram->area->obj_table);
}

void zram_rmap_erase(struct zram *zram, u32 index)
{
	unsigned long eswpentry;
	u32 ext_id;

	if (!zram) {
		hh_print(HHLOG_ERR, "NULL zram\n");
		return;
	}
	if (index >= (u32)zram->area->nr_objs) {
		hh_print(HHLOG_ERR, "index = %d invalid\n", index);
		return;
	}
#ifdef CONFIG_ZRAM_DEDUP
	eswpentry = zram_get_direct_handle(zram, index);
#else
	eswpentry = zram_get_handle(zram, index);
#endif
	ext_id = esentry_extid(eswpentry);
	hh_list_del(obj_idx(zram->area, index),
		ext_idx(zram->area, ext_id),
		zram->area->obj_table);
}
