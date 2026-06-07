/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2019. All rights reserved.
 * Description: This file describe HISI GPU hardware related features
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

#ifndef _MEMORY_GROUP_MANAGER_H_
#define _MEMORY_GROUP_MANAGER_H_

#include <mali_malisw.h>
#ifdef CONFIG_MALI_LAST_BUFFER
#include <linux/hisi/hisi_lb.h>
#endif

#if (KERNEL_VERSION(4, 17, 0) > LINUX_VERSION_CODE)
#define vm_fault_t int
#endif

#define MEMORY_GROUP_MANAGER_NR_GROUPS 16

/* Memory type of memory group manager */
enum memory_group_manager_memory_type {
	MEMORY_GROUP_MANAGER_MEMORY_TYPE_DMA_BUF
};

/* Import type and dma_buf of memory group manager */
struct memory_group_manager_import_data {
	enum memory_group_manager_memory_type type;
	struct dma_buf *dma_buf;
};

struct memory_group_manager_device;

/**
 * struct memory_group_manager_ops - Callbacks for memory group manager
 *                                   operations
 *
 * @mgm_alloc_page: Callback to allocate physical memory in a group
 * @mgm_free_page:  Callback to free physical memory in a group
 */
struct memory_group_manager_ops {
	/**
	 * mgm_alloc_page - Allocate a physical memory page in a group
	 *
	 * @mgm_dev:  The memory group manager through which the request is
	 *            being made.
	 * @group_id: A physical memory group ID. The meaning of this is defined
	 *            by the systems integrator. Its valid range is
	 *            0 .. MEMORY_GROUP_MANAGER_NR_GROUPS-1.
	 * @gfp_mask: Bitmask of Get Free Page flags affecting allocator
	 *            behavior.
	 * @order:    Page order for physical page size (order=0 means 4 KiB,
	 *            order=9 means 2 MiB).
	 *
	 * Return: Pointer to allocated page, or NULL if allocation failed.
	 */
	struct page *(*mgm_alloc_page)(
		struct memory_group_manager_device *mgm_dev, int group_id,
		gfp_t gfp_mask, unsigned int order);

	/**
	 * mgm_free_page - Free a physical memory page in a group
	 *
	 * @mgm_dev:  The memory group manager through which the request
	 *            is being made.
	 * @group_id: A physical memory group ID. The meaning of this is
	 *            defined by the systems integrator. Its valid range is
	 *            0 .. MEMORY_GROUP_MANAGER_NR_GROUPS-1.
	 * @page:     Address of the struct associated with a page of physical
	 *            memory that was allocated by calling the mgm_alloc_page
	 *            method of the same memory pool with the same values of
	 *            @group_id and @order.
	 * @order:    Page order for physical page size (order=0 means 4 KiB,
	 *            order=9 means 2 MiB).
	 */
	void (*mgm_free_page)(
		struct memory_group_manager_device *mgm_dev, int group_id,
		struct page *page, unsigned int order);

	/**
	 * get_import_mem_id -- Get the physical memory group ID for the
	 *                      imported memory
	 *
	 * @mgm_dev: The memory group manager the request is being
	 *           made through.
	 * @data: Data which describes the imported memory.
	 *
	 * Note that provision of this call back is optional, where
	 * it is not provided. this call back pointer must be set to NULL
	 * to indicate it is not in use.
	 *
	 * Return: The memory group ID to use when mapping pages from this
	 *         imported memory.
	 */
	int (*get_import_mem_id)(struct memory_group_manager_device *mgm_dev,
		struct memory_group_manager_import_data *data);

	/**
	 * gpu_map_page -- Map a physical address to the GPU
	 *
	 * This function can be used to encode extra information into the
	 * GPU's page table entry.
	 * @mmu_level: The level the page table is being built for.
	 * @pte: The prebuilt page table entry from KBase, either in lpae
	 * or aarch64 depending on the drivers configuration. This should be
	 * decoded to determine the physical address and other properties of
	 * the mapping the manager requires.
	 *
	 * Return: A page table entry prot for KBase to store in the
	 * page tables.
	 */
	u64 (*gpu_map_page)(u8 mmu_level, u64 pte);

	/**
	 * vm_insert_pfn -- Map a physical address to the CPU
	 *
	 * Note: Unlike gpu_map_page this function must do the work of writing
	 * the CPU's page table entry.
	 * @mgm_dev: The memory group manager the request is being made through.
	 * @vma: The vma the physical address is being is being mapped into.
	 * @addr: The address to map to.
	 * @pfn: The Page Frame Number to map.
	 */
	vm_fault_t (*vm_insert_pfn)(struct memory_group_manager_device *mgm_dev,
		struct vm_area_struct *vma,
		unsigned long addr,
		unsigned long pfn);


	/**
	 * remap_vmalloc_range -- map vmalloc pages to userspace
	 *
	 * @mgm_dev: The memory group manager the request is being made through.
	 * @vma: vma to cover (map full range of vma).
	 * @addr: vmalloc memory.
	 * @pgoff: number of pages into addr before first page to map.
	 */
	int (*remap_vmalloc_range)(struct memory_group_manager_device *mgm_dev,
		struct vm_area_struct *vma, void *addr,
		unsigned long pgoff);


	/**
	 * vmap  - map an array of pages into virtually contiguous space
	 * @pages: array of page pointers
	 * @count: number of pages to map
	 * @offset: number of normal pages offset to the start of
	 * virtual space. pages[offset] is the beginning of the
	 * normal memory pages.
	 * @flags: vm_area->flags
	 * @prot: page protection for the mapping
	 *
	 * Maps @count pages from @pages into contiguous kernel virtual space.
	 */
	void *(*vmap)(struct memory_group_manager_device *mgm_dev,
		struct page **pages, unsigned int count,
		unsigned int offset,
		unsigned long flags, pgprot_t prot);
};

/**
 * struct memory_group_manager_device - Device structure for a memory group
 *                                      manager
 *
 * @ops  - Callbacks associated with this device
 * @data - Pointer to device private data
 *
 * In order for a systems integrator to provide custom behaviors for memory
 * operations performed by the kbase module (controller driver), they must
 * provide a platform-specific driver module which implements this interface.
 *
 * This structure should be registered with the platform device using
 * platform_set_drvdata().
 */
struct memory_group_manager_device {
	struct memory_group_manager_ops ops;
	void *data;
};

#endif /* _MEMORY_GROUP_MANAGER_H_ */
