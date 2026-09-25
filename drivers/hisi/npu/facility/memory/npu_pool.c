/*
 * npu_pool.c
 *
 * about npu pool
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include "npu_pool.h"

#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/vmalloc.h>
#include <securec.h>

#include "npu_log.h"

struct devdrv_pool_info {
	u8 is_created;
	vir_addr_t virt_addr;
	phys_addr_t phy_addr;
	unsigned long iova;
	u32 used_num;
	u32 entity_num;
} ;
/*
Pool allocation principle: You can only allocate pools in order from 0 to n, that is, when expanding,
		it will expand backwards, and when it shrinks, it will shrink from the back.
		Even if only the nth pool is used, the front is empty, and the previous pool will not be released first.
Id assignment principle: assign the currently available minimum id each time
*/
struct devdrv_pool_manager {
	u32 entity_num;
	u32 entity_size;
	u32 pool_num;
	alloc_pool_func alloc_pool;
	free_pool_func free_pool;
	struct devdrv_pool_info *pool;
	u32 pool_created;
	struct devdrv_entity_info *entity;
	struct list_head available_entity_list; // Represents a list of all available ids, arranged in ascending order of id
};

#define	DEVDRV_MEM_POOL_USE_THRESHOLD(size) ((size) * 4 / 5)

static struct devdrv_pool_manager g_pool_manager[NPU_DEV_NUM][DEVDRV_MEM_POOL_TYPE_MAX] = { { { 0 } } };
static spinlock_t g_pool_manager_spinlock;

int devdrv_pool_regist(u8 dev_id, tag_devdrv_mem_pool_type type, u32 entity_num, u32 entity_size, u8 pool_num,
                       alloc_pool_func alloc_pool, free_pool_func free_pool)
{
	struct devdrv_pool_manager *pool_manager = NULL;
	int i;
	int ret = 0;
	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id: %d\n", dev_id);
		return -1;
	}

	if (type >= DEVDRV_MEM_POOL_TYPE_MAX) {
		NPU_DRV_ERR("illegal type: %d\n", type);
		return -1;
	}

	if (pool_num < 1 || entity_num < pool_num || entity_num % pool_num != 0) {
		NPU_DRV_ERR("illegal info:entity_num=%d, entity_size=%d, pool_num=%d\n", entity_num, entity_size, pool_num);
		return -1;
	}

	if (alloc_pool == NULL || free_pool == NULL) {
		NPU_DRV_ERR("illegal func:alloc_pool=%pK, free_pool=%pK\n", alloc_pool, free_pool);
		return -1;
	}

	NPU_DRV_INFO("devdrv_pool_regist  enter:entity_num=%d, entity_size=%d, pool_num=%d\n",
		entity_num, entity_size, pool_num);
	pool_manager = &(g_pool_manager[dev_id][type]);
	spin_lock_init(&g_pool_manager_spinlock);
	pool_manager->entity_num = entity_num;
	pool_manager->entity_size = entity_size;
	pool_manager->pool_num = pool_num;
	pool_manager->alloc_pool = alloc_pool;
	pool_manager->free_pool = free_pool;

	// 1. init id list
	NPU_DRV_INFO("devdrv_pool_regist to alloc entities:size=%u * %u\n",
		sizeof(struct devdrv_entity_info), entity_num);
	pool_manager->entity = (struct devdrv_entity_info *)vmalloc(sizeof(struct devdrv_entity_info) * entity_num);
	if (pool_manager->entity == NULL) {
		NPU_DRV_ERR("no mem to alloc entity info table\n");
		return -ENOMEM;
	}
	memset_s(pool_manager->entity, sizeof(struct devdrv_entity_info) * entity_num, 0x0,
		sizeof(struct devdrv_entity_info) * entity_num);
	INIT_LIST_HEAD(&pool_manager->available_entity_list);
	for (i = 0; i < entity_num; i++) {
		pool_manager->entity[i].index = (u32)i;
		list_add_tail(&pool_manager->entity[i].list, &pool_manager->available_entity_list);
	}

	// 2. init pool info table
	NPU_DRV_INFO("devdrv_pool_regist  to alloc pools:size=%u * %u\n", sizeof(struct devdrv_pool_info), pool_num);
	pool_manager->pool = (struct devdrv_pool_info *)vmalloc(sizeof(struct devdrv_pool_info) * pool_num);
	if (pool_manager->pool == NULL) {
		NPU_DRV_ERR("no mem to alloc pool info table\n");
		ret = -ENOMEM;
		goto allocPoolinfoFail;
	}
	memset_s(pool_manager->pool, sizeof(struct devdrv_pool_info) * pool_num, 0x0,
		sizeof(struct devdrv_pool_info) * pool_num);
	for (i = 0; i < pool_num; i++)
		pool_manager->pool[i].entity_num = entity_num / pool_num;
	// alloc the first pool
	if (alloc_pool(pool_manager->pool[0].entity_num * entity_size,
		&pool_manager->pool[0].virt_addr, &pool_manager->pool[0].phy_addr,
		&pool_manager->pool[0].iova) != 0) {
		NPU_DRV_ERR("no mem to alloc the first pool\n");
		ret = -ENOMEM;
		goto allocPoolFail;
	}
	pool_manager->pool[0].is_created = 1;
	pool_manager->pool_created = 1;

	return 0;
allocPoolFail:
	vfree(pool_manager->pool);
allocPoolinfoFail:
	vfree(pool_manager->entity);
	return ret;
}
int devdrv_pool_unregist(u8 dev_id, tag_devdrv_mem_pool_type type)
{
	struct devdrv_pool_manager *pool_manager = NULL;
	int i;

	NPU_DRV_DEBUG("enter cur dev %d type %d\n", dev_id, type);

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id: %d\n", dev_id);
		return -1;
	}

	if (type >= DEVDRV_MEM_POOL_TYPE_MAX) {
		NPU_DRV_ERR("illegal type:%d\n", type);
		return -1;
	}

	pool_manager = &g_pool_manager[dev_id][type];
	spin_lock(&g_pool_manager_spinlock);
	// 1. free pool
	for (i = 0; i < pool_manager->pool_created; i++)
		pool_manager->free_pool(pool_manager->pool[i].virt_addr,
			pool_manager->pool[i].phy_addr, pool_manager->pool[i].iova);

	pool_manager->pool_created = 0;

	// 2. free pool info table
	vfree(pool_manager->pool);

	// 3. free entity info table
	vfree(pool_manager->entity);

	memset_s(&g_pool_manager[dev_id][type], sizeof(struct devdrv_pool_manager), 0, sizeof(struct devdrv_pool_manager));
	spin_unlock(&g_pool_manager_spinlock);
	return 0;
}

struct devdrv_entity_info *devdrv_alloc_entity(u8 dev_id, tag_devdrv_mem_pool_type type)
{
	struct devdrv_pool_manager *pool_manager = NULL;
	struct devdrv_entity_info *entity_info = NULL;
	int pool_index;
	int ret;
	u32 pool_size;
	u64 index_offset;

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id: %d\n", dev_id);
		return NULL;
	}

	if (type >= DEVDRV_MEM_POOL_TYPE_MAX) {
		NPU_DRV_ERR("illegal type:%d\n", type);
		return NULL;
	}

	pool_manager = &g_pool_manager[dev_id][type];
	if (pool_manager->entity_num == 0) {
		NPU_DRV_ERR("devdrv_alloc_entity failed, dev=%d type=%d has not registed!\n", dev_id, type);
		return NULL;
	}

	spin_lock(&g_pool_manager_spinlock);
	// 1. select the first entity info node from entity info list
	if (list_empty_careful(&pool_manager->available_entity_list)) {
		spin_unlock(&g_pool_manager_spinlock);
		NPU_DRV_ERR("cur dev %d type %d available entity list empty!!!\n", dev_id, type);
		return NULL;
	}
	entity_info = list_first_entry(&pool_manager->available_entity_list,
		struct devdrv_entity_info, list);

	/* There is no case where the pool to which the id belongs is not applied,
	  because the usage rate of the last pool exceeds 80%, it is necessary to apply for a new pool.
	*/
	pool_index = entity_info->index / pool_manager->pool[0].entity_num;

	// 2.  Check if you need to extend the pool
	if (pool_manager->pool_created < pool_manager->pool_num && pool_index >= pool_manager->pool_created - 1) {
		// there is any more pool to apply, and pool_index is the id of the last pool current.
		// Note: theoretically '>' is impossible, unless there was an expansion failure before
		if (pool_manager->pool[pool_index].used_num > 
			DEVDRV_MEM_POOL_USE_THRESHOLD(pool_manager->pool[pool_index].entity_num)) {
			pool_size = pool_manager->entity_size * pool_manager->pool[0].entity_num;
			ret = pool_manager->alloc_pool(pool_size,
				&pool_manager->pool[pool_index + 1].virt_addr,
				&pool_manager->pool[pool_index + 1].phy_addr,
				&pool_manager->pool[pool_index + 1].iova);
			if (ret != 0) {
				spin_unlock(&g_pool_manager_spinlock);
				NPU_DRV_ERR("no mem to alloc the %dth pool\n", pool_index + 1);
				return NULL;
			}
			pool_manager->pool[pool_index + 1].is_created = 1;
			pool_manager->pool[pool_index + 1].used_num = 0;
			pool_manager->pool_created++;
		}
	}

	// 3. Fill in the address information of the entity_info
	list_del(&entity_info->list);
	pool_manager->pool[pool_index].used_num++;
	index_offset = (entity_info->index % pool_manager->pool[pool_index].entity_num) *
		((u64)pool_manager->entity_size);
	entity_info->vir_addr = pool_manager->pool[pool_index].virt_addr + index_offset;
	entity_info->phy_addr = pool_manager->pool[pool_index].phy_addr + index_offset;
	entity_info->iova = pool_manager->pool[pool_index].iova + index_offset;

	spin_unlock(&g_pool_manager_spinlock);

	NPU_DRV_DEBUG("cur dev %d type %d pool %d used %d entity\n", dev_id, type, pool_index,
		pool_manager->pool[pool_index].used_num);
	NPU_DRV_DEBUG("pool %d virt=%llx, phya=%llx, iova=%llx\n", pool_index, pool_manager->pool[pool_index].virt_addr,
		pool_manager->pool[pool_index].phy_addr, pool_manager->pool[pool_index].iova);
	NPU_DRV_DEBUG("pool entity num=%d, entity size=%d\n", pool_manager->pool[pool_index].entity_num,
		pool_manager->entity_size);
	NPU_DRV_DEBUG("alloc entity=%d, vir_addr=%llx, phya=%llx, iova=%llx\n", entity_info->index,
		entity_info->vir_addr, entity_info->phy_addr, entity_info->iova);
	return entity_info;
}

int devdrv_free_entity(u8 dev_id, tag_devdrv_mem_pool_type type, u32 index)
{
	struct devdrv_pool_manager *pool_manager = NULL;
	struct devdrv_entity_info *entity_info = NULL;
	struct devdrv_entity_info *entity_tmp1 = NULL;
	struct devdrv_entity_info *entity_tmp2 = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	int pool_index;

	NPU_DRV_DEBUG("enter cur dev %d type %d entity %d\n", dev_id, type, index);

	if (dev_id >= NPU_DEV_NUM) {
		NPU_DRV_ERR("illegal npu dev id: %d\n", dev_id);
		return -1;
	}

	if (type >= DEVDRV_MEM_POOL_TYPE_MAX) {
		NPU_DRV_ERR("illegal type:%d\n", type);
		return -1;
	}

	pool_manager = &g_pool_manager[dev_id][type];
	if (pool_manager->entity_num == 0) {
		NPU_DRV_ERR("devdrv_free_entity failed! dev=%d type=%d has not registed!\n", dev_id, type);
		return -1;
	}

	if (index >= pool_manager->entity_num) {
		NPU_DRV_ERR("illegal entity %d\n", index);
		return -1;
	}
	entity_info = &pool_manager->entity[index];
	spin_lock(&g_pool_manager_spinlock);

// 1. Find the correct location and insert the id node
	if (list_empty_careful(&pool_manager->available_entity_list)) {
		list_add(&entity_info->list, &pool_manager->available_entity_list);
	} else {
		list_for_each_safe(pos, n, &pool_manager->available_entity_list) {
			entity_tmp1 = list_entry(pos, struct devdrv_entity_info, list);
			if (entity_tmp1 == NULL) {
				spin_unlock(&g_pool_manager_spinlock);
				NPU_DRV_ERR("entity is null\n");
				return -1;
			}
			// 1. insert list head--(e(pos) > index)
			if (entity_tmp1->index > index) {
				list_add(&entity_info->list, &pool_manager->available_entity_list);
				break;
			}
			// 2. insert list tail--(e(pos) < index && pos is tail)
			if (n == &pool_manager->available_entity_list) {
				list_add_tail(&entity_info->list, &pool_manager->available_entity_list);
				break;
			}
			// 3. insert between pos and n--(e(pos) < index && e(n) > index)
			entity_tmp2 = list_entry(n, struct devdrv_entity_info, list);
			if (entity_tmp2 == NULL) {
				spin_unlock(&g_pool_manager_spinlock);
				NPU_DRV_ERR("hwts sq sub is null\n");
				return -1;
			}
			if (entity_tmp2->index > index) {
				list_add(&entity_info->list, &entity_tmp1->list);
				break;
			}
			// 4. go to next--(e(pos) < index && e(n) < index)
		}
	}

// 2. update pool info
	pool_index = index / pool_manager->pool[0].entity_num;
	pool_manager->pool[pool_index].used_num--;

// 3. Check if you want to shrink the pool
	while (pool_manager->pool_created > 1) {
		if (pool_manager->pool[pool_manager->pool_created - 1].used_num != 0 ||
			pool_manager->pool[pool_manager->pool_created - 1].used_num >
			DEVDRV_MEM_POOL_USE_THRESHOLD(pool_manager->pool[0].entity_num))
			break;

		pool_manager->free_pool(pool_manager->pool[pool_manager->pool_created - 1].virt_addr,
			pool_manager->pool[pool_manager->pool_created - 1].phy_addr,
			pool_manager->pool[pool_manager->pool_created - 1].iova);
		pool_manager->pool[pool_manager->pool_created - 1].virt_addr = 0;
		pool_manager->pool[pool_manager->pool_created - 1].phy_addr = 0;
		pool_manager->pool[pool_manager->pool_created - 1].iova = 0;
		pool_manager->pool[pool_manager->pool_created - 1].is_created = 0;
		pool_manager->pool_created--;
	}

	spin_unlock(&g_pool_manager_spinlock);
	NPU_DRV_DEBUG("cur dev %d type %d pool %d used %d entity\n", dev_id, type, pool_index,
		pool_manager->pool[pool_index].used_num);
	return 0;
}

