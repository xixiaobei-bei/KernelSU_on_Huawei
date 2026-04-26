/*
 * hisi_drm_heaps_manager.c
 *
 * hisi drm memory manager. Init all register heap and init tracer
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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
#include "hisi_drm_heaps_manager.h"

#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/sched/signal.h>
#include <linux/time.h>
#include <linux/fdtable.h>

#include "hisi_drm_debug_heap.h"
#include "hisi_drm_page_pool_heap.h"
#include "hisi_drm_heaps_defs.h"
#include "hisi_drm_heaps_tracer_inner.h"

static LIST_HEAD(hisi_drm_heaps);

struct heaps_type_table {
	const char *name;
	enum hisi_drm_heap_type type;
};

static const struct heaps_type_table type_table[] = {
	{ "hisi_drm_page_pool", HISI_DRM_PAGE_POOL },
	{ "hisi_drm_debug_pool", HISI_DRM_DEBUG_POOL },
};

struct hisi_drm_heap *hisi_drm_get_heap(unsigned int heap_id)
{
	struct list_head *heap_head = &hisi_drm_heaps;
	struct hisi_drm_heap *pos = NULL;

	list_for_each_entry (pos, heap_head, heap_list) {
		if (heap_id == pos->heap_id)
			return pos;
	}

	return ERR_PTR(-EINVAL);
}

static unsigned long hisi_drm_heap_shrink_count(struct shrinker *shrinker,
						struct shrink_control *sc)
{
	struct hisi_drm_heap *heap =
		container_of(shrinker, struct hisi_drm_heap, shrinker);
	int total = 0;

	if (heap->ops->shrink)
		total += heap->ops->shrink(heap, sc->gfp_mask, 0);
	return total;
}

static unsigned long hisi_drm_heap_shrink_scan(struct shrinker *shrinker,
					       struct shrink_control *sc)
{
	struct hisi_drm_heap *heap =
		container_of(shrinker, struct hisi_drm_heap, shrinker);
	int freed = 0;
	int to_scan = (int)sc->nr_to_scan;

	if (to_scan == 0)
		return 0;

	to_scan -= freed;
	if (to_scan <= 0)
		return freed;

	if (heap->ops->shrink)
		freed += heap->ops->shrink(heap, sc->gfp_mask, to_scan);
	return freed;
}

static void hisi_drm_heap_init_shrinker(struct hisi_drm_heap *heap)
{
	int ret;
	heap->shrinker.count_objects = hisi_drm_heap_shrink_count;
	heap->shrinker.scan_objects = hisi_drm_heap_shrink_scan;
	heap->shrinker.seeks = DEFAULT_SEEKS;
	heap->shrinker.batch = 0;
	ret = register_shrinker(&heap->shrinker);
	if (ret)
		pr_heaps_warning(
				"register shrinker for heap[%s] failed, ret=%d\n",
				heap->name, ret);
}

/**
 * Save heap into list, if heap implement shrink, register it as shrinker.
 * All heap will register the heap into debugfs with it's heap name.
 */
static void hisi_drm_heaps_add_heap(struct hisi_drm_heap *heap)
{
	struct hisi_drm_heap *pos = NULL;

	if (!heap->ops || !heap->ops->alloc_buf || !heap->ops->free_buf)
		return;

	if (heap->ops->shrink)
		hisi_drm_heap_init_shrinker(heap);

	list_for_each_entry (pos, &hisi_drm_heaps, heap_list) {
		if (unlikely(pos->heap_id == heap->heap_id)) {
			pr_heaps_err(
				"the heap_id is already exist, please check\n");
			return;
		}
	}

	list_add(&heap->heap_list, &hisi_drm_heaps);

	hisi_drm_heaps_init_heap_debugfs(heap);
}

/**
 * Search for type table, if table's name is equal with DTS' name,
 * it is matched, return the table's type.
 * @type_name the name that want to compare
 * @type the type which we want to get
 */
static int get_type_by_name(const char *type_name,
			    enum hisi_drm_heap_type *type)
{
	unsigned int i;
	unsigned int array_size = ARRAY_SIZE(type_table);
	unsigned int len = strlen(type_name);

	for (i = 0; i < array_size; ++i) {
		if (strncmp(type_name, type_table[i].name, len))
			continue;
		*type = type_table[i].type;
		return 0;
	}

	return -EINVAL;
}

/*
 * Set heap's properties by read from dts
 */
static int set_heap_property(const char *heap_name, struct device_node *dt_node,
			     struct platform_device *pdev,
			     struct hisi_drm_platform_heap *heap)
{
	unsigned int size;
	unsigned int base;
	unsigned int id;
	int ret;
	const char *type_name = NULL;
	enum hisi_drm_heap_type type;

	ret = of_property_read_u32(dt_node, "heap-id", &id);
	if (ret) {
		pr_heaps_err("check id failed\n");
		return ret;
	}

	ret = of_property_read_u32(dt_node, "heap-base", &base);
	if (ret)
		pr_heaps_debug("can't find heap-base property\n");

	ret = of_property_read_u32(dt_node, "heap-size", &size);
	if (ret)
		pr_heaps_debug("can't find heap-size property\n");

	ret = of_property_read_string(dt_node, "heap-type", &type_name);
	if (ret) {
		pr_heaps_err("check type name failed\n");
		return ret;
	}
	ret = get_type_by_name(type_name, &type);
	if (ret) {
		pr_heaps_err("type name parse failed\n");
		return ret;
	}

	heap->name = heap_name;
	heap->type = type;
	heap->id = id;
	heap->size = size;
	heap->base = base;
	heap->priv = (void *)&pdev->dev;

	return 0;
}

static struct hisi_drm_heap *
hisi_drm_heap_create(struct hisi_drm_platform_heap *heap_data)
{
	struct hisi_drm_heap *heap = NULL;

	if (unlikely(!heap_data)) {
		pr_heaps_err("invalid heap\n");
		return ERR_PTR(-EINVAL);
	}

	switch (heap_data->type) {
	case HISI_DRM_PAGE_POOL:
		heap = hisi_drm_page_pool_heap_create(heap_data);
		break;
	case HISI_DRM_DEBUG_POOL:
		heap = hisi_drm_debug_heap_create(heap_data);
		break;
	default:
		pr_heaps_err("unknown heap type=%#x\n", heap_data->type);
		break;
	}

	if (IS_ERR_OR_NULL(heap))
		return ERR_PTR(-EINVAL);

	heap->name = heap_data->name;
	heap->heap_id = heap_data->id;

	return heap;
}

static void of_create_heap(struct hisi_drm_platform_heap *pheap)
{
	struct hisi_drm_heap *heap = NULL;

	heap = hisi_drm_heap_create(pheap);
	if (IS_ERR_OR_NULL(heap)) {
		pr_heaps_err(
			"err create[%20s]--[id:%d]--[type:%d]--[base:%#lx @ size:%#lx]\n",
			pheap->name, pheap->id, pheap->type,
			(unsigned long)pheap->base, (unsigned long)pheap->size);
		return;
	}

	pr_heaps_info(
		"success create[%20s]--[id:%d]--[type:%d]--[base:%#lx @ size:%#lx]\n",
		pheap->name, pheap->id, pheap->type, (unsigned long)pheap->base,
		(unsigned long)pheap->size);

	hisi_drm_heaps_add_heap(heap);
}

/**
 * Get heap info by read DTS properties, then set info into platform_heap by
 * call @set_heap_property, when get all useful info, then, create heap by call
 * @of_create_heap
 */
static int of_parse_drm_dts(struct platform_device *pdev,
			    struct device_node *dt_node)
{
	const char *heap_name = NULL;
	const char *status = NULL;
	int ret;
	struct device_node *np = NULL;
	struct hisi_drm_platform_heap *tmp_heap = NULL;

	for_each_child_of_node (dt_node, np) {

		/* if status is set ok or not set, it means avaliable. */
		ret = of_property_read_string(np, "status", &status);
		if (!ret) {
			if (strncmp("ok", status, strlen("ok")))
				continue;
		}

		ret = of_property_read_string(np, "heap-name", &heap_name);
		if (ret) {
			pr_heaps_err("can not get heap-name in node\n");
			continue;
		}

		tmp_heap =
			devm_kzalloc(&pdev->dev, sizeof(*tmp_heap), GFP_KERNEL);
		if (!tmp_heap)
			return -ENOMEM;

		ret = set_heap_property(heap_name, np, pdev, tmp_heap);
		if (ret) {
			devm_kfree(&pdev->dev, tmp_heap);
			continue;
		}

		tmp_heap->node = np;

		of_create_heap(tmp_heap);
	}

	return 0;
}

static int do_init_heaps(struct platform_device *pdev)
{
	struct device_node *dt_node = pdev->dev.of_node;
	int ret;

	ret = of_parse_drm_dts(pdev, dt_node);
	if (ret) {
		pr_heaps_err("failed to parse drm heaps\n");
		return ret;
	}

	return 0;
}

/**
 * When DRM heaps manager probe, we will init the debugger and register
 * heaps by read DTS properties.
 */
static int hisi_drm_heaps_probe(struct platform_device *pdev)
{
	int ret;

	ret = hisi_drm_heaps_tracer_init();
	if (ret)
		pr_heaps_info("failed to create debug\n");

	ret = do_init_heaps(pdev);
	if (ret)
		return ret;

	return 0;
}

static const struct of_device_id hisi_drm_heaps_match_table[] = {
	{ .compatible = "hisilicon, hisi-drm-heaps" },
	{},
};

static struct platform_driver hisi_drm_heaps_driver = {
	.driver = {
		.name = "drm-heaps-hisi",
		.of_match_table = hisi_drm_heaps_match_table,
	},

	.probe = hisi_drm_heaps_probe,
};

static int __init hisi_drm_heaps_init(void)
{
	int ret = platform_driver_register(&hisi_drm_heaps_driver);

	return ret;
}

subsys_initcall_sync(hisi_drm_heaps_init)
