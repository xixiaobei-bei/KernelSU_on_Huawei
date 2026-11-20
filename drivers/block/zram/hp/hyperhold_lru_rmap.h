/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description: hyperhold header file
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
#ifndef _HYPERHOLD_LRU_MAP_
#define _HYPERHOLD_LRU_MAP_

#define EXTENT_MAX_OBJ_CNT (30 * EXTENT_PG_CNT)

void zram_set_memcg(struct zram *zram, u32 index, struct mem_cgroup *mcg);
struct mem_cgroup *zram_get_memcg(struct zram *zram, u32 index);
int zram_get_memcg_coldest_index(struct hyperhold_area *area,
				 struct mem_cgroup *mcg,
				 int *index, int max_cnt);
int zram_rmap_get_extent_index(struct hyperhold_area *area,
			       int ext_id, int *index);
void zram_lru_add(struct zram *zram, u32 index, struct mem_cgroup *mcg);
void zram_lru_add_tail(struct zram *zram, u32 index, struct mem_cgroup *mcg);
void zram_lru_del(struct zram *zram, u32 index);
void zram_rmap_insert(struct zram *zram, u32 index);
void zram_rmap_erase(struct zram *zram, u32 index);

#endif
