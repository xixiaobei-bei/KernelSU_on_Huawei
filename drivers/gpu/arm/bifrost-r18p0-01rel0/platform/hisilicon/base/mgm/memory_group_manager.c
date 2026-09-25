/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2019. All rights reserved.
 * Description: This file describe HISI GPU related init
 * Author: Huawei Technologies Co., Ltd.
 * Create: 2014-2-24
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * SPDX-License-Identifier: GPL-2.0
 *
 */

#include <asm/pgtable.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#endif
#include "memory_group_manager.h"

/**
 * struct mgm_group - Structure to keep track of the number of allocated
 *                    pages per group
 *
 * @size:  The number of allocated small(4KB) pages
 * @lp_size:  The number of allocated large(2MB) pages
 *
 * This structure allows page allocation information to be displayed via
 * debugfs. Display is organized per group with small and large sized pages.
 */
struct mgm_group {
	size_t size;
	size_t lp_size;
};

/**
 * struct mgm_groups - Structure for groups of memory group manager
 *
 * @groups: To keep track of the number of allocated pages of all groups
 * @dev: device attached
 * @mgm_debugfs_root: debugfs root directory of memory group manager
 *
 * This structure allows page allocation information to be displayed via
 * debugfs. Display is organized per group with small and large sized pages.
 */
struct mgm_groups {
	spinlock_t groups_lock;
	struct mgm_group groups[MEMORY_GROUP_MANAGER_NR_GROUPS];
	struct device *dev;
#ifdef CONFIG_DEBUG_FS
	struct dentry *mgm_debugfs_root;
#endif
};

#ifdef CONFIG_DEBUG_FS

static int mgm_size_get(void *data, u64 *val)
{
	struct mgm_group *group = data;

	*val = group->size;

	return 0;
}

static int mgm_lp_size_get(void *data, u64 *val)
{
	struct mgm_group *group = data;

	*val = group->lp_size;

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(fops_mgm_size, mgm_size_get, NULL, "%llu\n");
DEFINE_SIMPLE_ATTRIBUTE(fops_mgm_lp_size, mgm_lp_size_get, NULL, "%llu\n");

static void mgm_term_debugfs(struct mgm_groups *data)
{
	debugfs_remove_recursive(data->mgm_debugfs_root);
}

#define MGM_DEBUGFS_GROUP_NAME_MAX 10
static int mgm_initialize_debugfs(struct mgm_groups *mgm_data)
{
	int i;
	struct dentry *e = NULL;
	struct dentry *g = NULL;
	char debugfs_group_name[MGM_DEBUGFS_GROUP_NAME_MAX] = { 0 };

	/*
	 * Create root directory of memory-group-manager
	 */
	mgm_data->mgm_debugfs_root =
		debugfs_create_dir("physical-memory-group-manager", NULL);
	if (IS_ERR(mgm_data->mgm_debugfs_root)) {
		dev_err(mgm_data->dev, "fail to create debugfs root directory\n");
		return -ENODEV;
	}

	/*
	 * Create debugfs files per group
	 */
	for (i = 0; i < MEMORY_GROUP_MANAGER_NR_GROUPS; i++) {
		scnprintf(debugfs_group_name, MGM_DEBUGFS_GROUP_NAME_MAX,
				"group_%d", i);
		g = debugfs_create_dir(debugfs_group_name,
				mgm_data->mgm_debugfs_root);
		if (IS_ERR(g)) {
			dev_err(mgm_data->dev, "fail to create group[%d]\n", i);
			goto remove_debugfs;
		}

		/* 0444 for file read-only */
		e = debugfs_create_file("size", 0444, g, &mgm_data->groups[i],
				&fops_mgm_size);
		if (IS_ERR(e)) {
			dev_err(mgm_data->dev, "fail to create size[%d]\n", i);
			goto remove_debugfs;
		}

		/* 0444 for file read-only */
		e = debugfs_create_file("lp_size", 0444, g,
				&mgm_data->groups[i], &fops_mgm_lp_size);
		if (IS_ERR(e)) {
			dev_err(mgm_data->dev,
				"fail to create lp_size[%d]\n", i);
			goto remove_debugfs;
		}
	}

	return 0;

remove_debugfs:
	mgm_term_debugfs(mgm_data);
	return -ENODEV;
}

#else

static void mgm_term_debugfs(struct mgm_groups *data)
{
}

static int mgm_initialize_debugfs(struct mgm_groups *mgm_data)
{
	return 0;
}

#endif /* CONFIG_DEBUG_FS */

#define ORDER_SMALL_PAGE 0
#define ORDER_LARGE_PAGE 9
static void update_size(struct memory_group_manager_device *mgm_dev, int
		group_id, int order, bool alloc)
{
	struct mgm_groups *data = mgm_dev->data;

	spin_lock(&data->groups_lock);
	switch (order) {
	case ORDER_SMALL_PAGE:
		if (alloc) {
			data->groups[group_id].size++;
		} else {
			WARN_ON(data->groups[group_id].size == 0);
			data->groups[group_id].size--;
		}
	break;

	case ORDER_LARGE_PAGE:
		if (alloc) {
			data->groups[group_id].lp_size++;
		} else {
			WARN_ON(data->groups[group_id].lp_size == 0);
			data->groups[group_id].lp_size--;
		}
	break;

	default:
		dev_err(data->dev, "Unknown order(%d)\n", order);
	break;
	}
	spin_unlock(&data->groups_lock);
}

static struct page *hisi_mgm_alloc_page(
	struct memory_group_manager_device *mgm_dev, int group_id,
	gfp_t gfp_mask, unsigned int order)
{
	struct page *p = NULL;
	struct mgm_groups *data = mgm_dev->data;

	WARN_ON(group_id < 0);
	WARN_ON(group_id >= MEMORY_GROUP_MANAGER_NR_GROUPS);

	p = alloc_pages(gfp_mask, order);

	if (p)
		update_size(mgm_dev, group_id, order, true);
	else
		dev_err(data->dev, "alloc_pages failed\n");

#ifdef CONFIG_MALI_LAST_BUFFER
	/* We attach the page here if it is allocated from last buffer */
	if (group_id && lb_pages_attach(group_id, p, 1ULL << order))
		/* If the page allocated from last buffer
		 * is used as normal page,
		 * it will cause unpredictable issue.
		 */
		dev_err(data->dev,
			"attach the page with last buffer failed. group_id[%d] order[%u]\n",
			group_id, order);
#endif

	return p;
}

static void hisi_mgm_free_page(
	struct memory_group_manager_device *mgm_dev, int group_id,
	struct page *page, unsigned int order)
{
#ifdef CONFIG_MALI_LAST_BUFFER
	struct mgm_groups *data = mgm_dev->data;
#endif

	WARN_ON(group_id < 0);
	WARN_ON(group_id >= MEMORY_GROUP_MANAGER_NR_GROUPS);

#ifdef CONFIG_MALI_LAST_BUFFER
	/* We dettach the page here if it is allocated from last buffer */
	if (group_id && lb_pages_detach(group_id, page, 1ULL << order))
		/* If the page allocated from last buffer cannot be dettached,
		 * it will cause the cpu virtual memory leak issue.
		 */
		dev_err(data->dev,
			"dettach the page from last buffer failed. group_id[%d] order[%u]\n",
			group_id, order);
#endif

	__free_pages(page, order);

	update_size(mgm_dev, group_id, order, false);
}

static void hisi_mgm_update_sizes(
                    struct memory_group_manager_device *mgm_dev, int group_id,
                    unsigned int order, bool is_alloc)
{
	update_size(mgm_dev, group_id, order, is_alloc);
}

static int hisi_mgm_get_import_mem_id(
		struct memory_group_manager_device *mgm_dev,
		struct memory_group_manager_import_data *data)
{
	CSTD_UNUSED(mgm_dev);
	CSTD_UNUSED(data);

	/* Currently, the last buffer doesn't support import memory,
	 * so directly return here.
	 */
	return 0;
}

static u64 hisi_mgm_gpu_map_page(u8 mmu_level, u64 pte)
{
	CSTD_UNUSED(mmu_level);

#ifdef CONFIG_MALI_LAST_BUFFER
	/* L.B intergration: Get the GID info.
	 * 1. In order to get the phys's GID, we will call hisi-driver function
	 *    lb_pte_attr(phy) here. like:
	 *    u64 phy_gid = lb_pte_attr(as_phys_addr_t(phy));
	 *
	 * 2. Call page_table_entry_set with the created phy_gid instead of phy.
	 */
	return lb_pte_attr(pte);
#else
	/* Nothing to do. */
	return 0;
#endif
}

#if (KERNEL_VERSION(4, 17, 0) > LINUX_VERSION_CODE)
#define vm_fault_t int

static inline vm_fault_t vmf_insert_pfn_prot(struct vm_area_struct *vma,
			unsigned long addr, unsigned long pfn, pgprot_t pgprot)
{
	int err = vm_insert_pfn_prot(vma, addr, pfn, pgprot);

	if (unlikely(err == -ENOMEM))
		return VM_FAULT_OOM;
	if (unlikely(err < 0 && err != -EBUSY))
		return VM_FAULT_SIGBUS;

	return VM_FAULT_NOPAGE;
}
#endif

static vm_fault_t hisi_mgm_vm_insert_pfn(
		struct memory_group_manager_device *mgm_dev,
		struct vm_area_struct *vma,
		unsigned long addr,
		unsigned long pfn)
{
#ifdef CONFIG_MALI_LAST_BUFFER
	struct page *gpu_page = NULL;
	unsigned int group_id;
	struct mgm_groups *data = mgm_dev->data;
#endif
	pgprot_t pgprot = vma->vm_page_prot;

#ifdef CONFIG_MALI_LAST_BUFFER
	gpu_page = pfn_to_page(pfn);

	BUG_ON(!gpu_page);
	group_id = lb_page_to_gid(gpu_page);

	/* Build the last buffer page with prot. */
	if (group_id && lb_prot_build(gpu_page, &pgprot)) {
		dev_err(data->dev,
			"Build last buffer's page prot failed.group_id[%u]\n",
			group_id);
		return -EINVAL;
	}
#endif

	return vmf_insert_pfn_prot(vma, addr, pfn, pgprot);
}


static int hisi_mgm_remap_vmalloc_range(
		struct memory_group_manager_device *mgm_dev,
		struct vm_area_struct *vma, void *addr,
		unsigned long pgoff)
{
	int err;

#ifdef CONFIG_MALI_LAST_BUFFER
	unsigned int group_id;
	struct mgm_groups *data = mgm_dev->data;
	struct page *gpu_page = vmalloc_to_page(addr);

	BUG_ON(!gpu_page);
	group_id = lb_page_to_gid(gpu_page);

	/* Build the last buffer page with prot. */
	if (group_id && lb_prot_build(gpu_page, &vma->vm_page_prot)) {
		dev_err(data->dev,
			"Build last buffer's vmalloc page prot failed.group_id[%u]\n",
			group_id);
		return -EINVAL;
	}
#endif

	err = remap_vmalloc_range(vma, addr, pgoff);

#ifdef CONFIG_MALI_LAST_BUFFER
	/* Clear the vma lb prot. */
	vma->vm_page_prot = pgprot_lb(vma->vm_page_prot, 0);
#endif

	return err;
}

static void *hisi_mgm_vmap(struct memory_group_manager_device *mgm_dev,
		struct page **pages, unsigned int count, unsigned int offset,
		unsigned long flags, pgprot_t prot)
{
	CSTD_UNUSED(mgm_dev);

#ifdef CONFIG_MALI_LAST_BUFFER
	/* Call lb_vmap() to support the split of memory. That is, the header
	 * of the memory is routes last buffer, however, the body doesn't.
	 * In this case, we have to allocate a continously virtual area and
	 * map the header & body seperately. The param offset is the bounds
	 * of the header & body.
	 */
	return lb_vmap(pages, count, offset, flags, prot);
#else
	CSTD_UNUSED(offset);
	return vmap(pages, count, flags, prot);
#endif
}

static int mgm_initialize_data(struct mgm_groups *mgm_data)
{
	int i;

	spin_lock_init(&mgm_data->groups_lock);
	for (i = 0; i < MEMORY_GROUP_MANAGER_NR_GROUPS; i++) {
		mgm_data->groups[i].size = 0;
		mgm_data->groups[i].lp_size = 0;
	}

	return mgm_initialize_debugfs(mgm_data);
}

static void mgm_term_data(struct mgm_groups *data)
{
	int i;

	for (i = 0; i < MEMORY_GROUP_MANAGER_NR_GROUPS; i++) {
		if (data->groups[i].size != 0)
			dev_warn(data->dev,
				"%zu 0-order pages in group(%d) leaked\n",
				data->groups[i].size, i);
		if (data->groups[i].lp_size != 0)
			dev_warn(data->dev,
				"%zu 9 order pages in group(%d) leaked\n",
				data->groups[i].lp_size, i);
	}

	mgm_term_debugfs(data);
}

static int memory_group_manager_probe(struct platform_device *pdev)
{
	struct memory_group_manager_device *mgm_dev = NULL;
	struct mgm_groups *mgm_data = NULL;

	mgm_dev = kzalloc(sizeof(*mgm_dev), GFP_KERNEL);
	if (!mgm_dev)
		return -ENOMEM;

	mgm_dev->ops.mgm_alloc_page = hisi_mgm_alloc_page;
	mgm_dev->ops.mgm_free_page = hisi_mgm_free_page;
	mgm_dev->ops.get_import_mem_id = hisi_mgm_get_import_mem_id;
	mgm_dev->ops.gpu_map_page = hisi_mgm_gpu_map_page;
	mgm_dev->ops.vm_insert_pfn = hisi_mgm_vm_insert_pfn;
	mgm_dev->ops.remap_vmalloc_range = hisi_mgm_remap_vmalloc_range;
	mgm_dev->ops.vmap = hisi_mgm_vmap;
	mgm_dev->ops.mgm_update_sizes = hisi_mgm_update_sizes;

	mgm_data = kzalloc(sizeof(*mgm_data), GFP_KERNEL);
	if (!mgm_data) {
		kfree(mgm_dev);
		return -ENOMEM;
	}

	mgm_dev->data = mgm_data;
	mgm_data->dev = &pdev->dev;

	if (mgm_initialize_data(mgm_data)) {
		kfree(mgm_data);
		kfree(mgm_dev);
		return -ENOENT;
	}

	platform_set_drvdata(pdev, mgm_dev);
	dev_info(&pdev->dev, "Memory group manager probed successfully\n");

	return 0; //lint !e429
}

static int memory_group_manager_remove(struct platform_device *pdev)
{
	struct memory_group_manager_device *mgm_dev =
		platform_get_drvdata(pdev);
	struct mgm_groups *mgm_data = mgm_dev->data;

	mgm_term_data(mgm_data);
	kfree(mgm_data);

	kfree(mgm_dev);

	dev_info(&pdev->dev, "Memory group manager removed successfully\n");

	return 0;
}

static const struct of_device_id memory_group_manager_dt_ids[] = {
	{ .compatible = "arm,physical-memory-group-manager" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, memory_group_manager_dt_ids);

static struct platform_driver memory_group_manager_driver = {
	.probe = memory_group_manager_probe,
	.remove = memory_group_manager_remove,
	.driver = {
		.name = "physical-memory-group-manager",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(memory_group_manager_dt_ids),
	}
};

static int __init memory_group_manager_driver_init(void)
{
	int ret;

	ret = platform_driver_register(&memory_group_manager_driver);
	if (ret)
		pr_err("[mali] memory_group_manager_driver init failed!\n");

	return ret;
}

rootfs_initcall(memory_group_manager_driver_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ARM Ltd.");
MODULE_VERSION("1.0");
