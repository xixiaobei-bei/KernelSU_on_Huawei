/*
 * Copyright (C) 2020 Hisilicon, Inc.
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
 */

#define pr_fmt(fmt) "cma_heap: " fmt

#include <linux/atomic.h>
#include <linux/cma.h>
#include <linux/of.h>
#include <linux/sizes.h>
#include <linux/slab.h>
#include <linux/hisi/hisi_ion.h>

#include "ion.h"
#include "ion_mm_cma_heap.h"

static int ion_add_mm_cma_heap(struct cma *cma, void *data)
{
	struct ion_cma_heap *cma_heap = (struct ion_cma_heap *)data;

	if (!cma_heap || !cma) {
		pr_err("cma_heap or cma is NULL\n");
		return 0;
	}

	if (strcmp(cma_get_name(cma), cma_heap->cma_name)) {
		pr_err("[%s] cma name: %s or %s\n", __func__,
			cma_get_name(cma), cma_heap->cma_name);
		return 0;
	}

	cma_heap->cma = cma;
	return 1;
}

static int ion_mm_cma_heap_parse_dt(struct device *dev,
			    struct ion_platform_heap *heap_data,
			    struct ion_cma_heap *cma_heap)
{
	struct device_node *nd = NULL;
	const char *compatible = NULL;
	int ret = 0;

	nd = of_get_child_by_name(dev->of_node, heap_data->name);
	if (!nd) {
		pr_err("can't of_get_child_by_name %s\n", heap_data->name);
		ret = -EINVAL;
		goto out;
	}

	ret = of_property_read_string(nd, "cma-name",
			&compatible);
	if (ret < 0) {
		pr_err("invalid cma-name in node [%s].\n",
			nd->name);
		ret = -EINVAL;
		goto out;
	}

	cma_heap->cma_name = compatible;

out:
	return ret;
}

static int ion_mm_cma_heap_prop(struct ion_cma_heap *cma_heap,
				struct device **dev,
				struct ion_platform_heap *heap_data)
{
	cma_heap->heap.type = heap_data->type;
	cma_heap->heap_size = heap_data->size;
	*dev = heap_data->priv;

	ion_mm_cma_heap_ops(cma_heap);

	if (ion_mm_cma_heap_parse_dt(*dev, heap_data, cma_heap))
		return -EINVAL;

	if (!cma_for_each_area(ion_add_mm_cma_heap, cma_heap))
		return -EINVAL;

	return 0;
}

struct ion_heap *ion_mm_cma_heap_create(struct ion_platform_heap *heap_data)
{
	struct ion_cma_heap *cma_heap = NULL;
	struct device *dev = NULL;
	int ret;

	cma_heap = kzalloc(sizeof(*cma_heap), GFP_KERNEL);
	if (!cma_heap)
		return ERR_PTR(-ENOMEM);

	ret = ion_mm_cma_heap_prop(cma_heap, &dev, heap_data);
	if (ret)
		goto free_heap;

	pr_err("mm cma heap info %s:\n"
		  "\t\t\t\t\t\t\t heap id : %u\n"
		  "\t\t\t\t\t\t\t heap size : %lu MB\n"
		  "\t\t\t\t\t\t\t cma base : 0x%llx\n"
		  "\t\t\t\t\t\t\t cma size : 0x%lx\n",
		  heap_data->name,
		  heap_data->id,
		  cma_heap->heap_size / SZ_1M,
		  cma_get_base(cma_heap->cma),
		  cma_get_size(cma_heap->cma));

	return &cma_heap->heap;

free_heap:
	kfree(cma_heap);
	return ERR_PTR(-ENOMEM);
}
