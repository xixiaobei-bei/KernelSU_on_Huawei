/*
 * tz_pm.c
 *
 * function for proc open,close session and invoke
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "tz_pm.h"
#include <securec.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>
#include <asm/compiler.h>
#include <asm/cacheflush.h>
#include "tc_ns_client.h"
#include "teek_ns_client.h"
#include "tc_ns_log.h"

#define S4_ADDR_4G              0xffffffff
#define HISI_RESERVED_SECOS_PHYMEM_BASE                  0x22800000
#define HISI_RESERVED_SECOS_PHYMEM_SIZE                  0x3000000
#define HISI_RESERVED_SECOS_S4_BASE                      0x27760000
#define HISI_RESERVED_SECOS_S4_SIZE                      0x100000

static char *g_s4_kernel_mem_addr;
static char *g_s4_buffer_vaddr;
static uint64_t g_s4_buffer_paddr;
static uint32_t g_s4_buffer_size;

static void *tc_vmap(phys_addr_t paddr, size_t size)
{
	uint32_t i;
	void *vaddr = NULL;
	pgprot_t pgprot = PAGE_KERNEL;
	uintptr_t offset;
	uint32_t pages_count;
	struct page **pages = NULL;

	offset = paddr & ~PAGE_MASK;
	paddr &= PAGE_MASK;
	pages_count = PAGE_ALIGN(size + offset) / PAGE_SIZE;

	pages = kzalloc(sizeof(struct page *) * pages_count, GFP_KERNEL);
	if (pages == NULL)
		return NULL;

	for (i = 0; i < pages_count; i++)
		*(pages + i) = phys_to_page((uintptr_t)(paddr + PAGE_SIZE * i));

	vaddr = vmap(pages, pages_count, VM_MAP, pgprot);
	kfree(pages);
	if (vaddr == NULL)
		return NULL;

	return offset + (char *)vaddr;
}

static int tc_s4_alloc_crypto_buffer(struct device *dev,
	 char **kernel_mem_addr)
{
	if (HISI_RESERVED_SECOS_S4_BASE > S4_ADDR_4G) {
		tloge("addr is invalid\n");
		return -EFAULT;
	}

	g_s4_buffer_vaddr = tc_vmap(HISI_RESERVED_SECOS_S4_BASE, HISI_RESERVED_SECOS_S4_SIZE);
	if (g_s4_buffer_vaddr == NULL) {
		tloge("vmap failed for s4\n");
		return -EFAULT;
	}
	g_s4_buffer_paddr = HISI_RESERVED_SECOS_S4_BASE;
	g_s4_buffer_size = HISI_RESERVED_SECOS_S4_SIZE;

	*kernel_mem_addr = vmalloc(HISI_RESERVED_SECOS_PHYMEM_SIZE);
	if (*kernel_mem_addr == NULL) {
		vunmap(g_s4_buffer_vaddr);
		g_s4_buffer_paddr = 0;
		g_s4_buffer_vaddr = NULL;
		g_s4_buffer_size = 0;
		tloge("vmalloc failed for s4\n");
		return -ENOMEM;
	}

	return 0;
}

static uint64_t tc_s4_suspend_or_resume(uint32_t power_op)
{
	register u64 smc_id asm("x0") = (u64)power_op;

	do {
		asm volatile (
			__asmeq("%0", "x0")
			"smc #0\n" :
			"+r"(smc_id)
			);
	} while (0);

	return smc_id;
}

static uint64_t tc_s4_crypto_and_copy(uint32_t crypt_op,
	uint64_t middle_mem_addr,
	uintptr_t secos_mem,
	uint32_t size, uint32_t index)
{
	register u64 smc_id asm("x0") = (u64)crypt_op;
	register u64 arg0 asm("x1") = (u64)middle_mem_addr;
	register u64 arg1 asm("x2") = (u64)secos_mem;
	register u64 arg2 asm("x3") = (u64)size;
	register u64 arg3 asm("x4") = (u64)index;

	do {
		asm volatile (
			__asmeq("%0", "x0")
			__asmeq("%1", "x1")
			__asmeq("%2", "x2")
			__asmeq("%3", "x3")
			__asmeq("%4", "x4")
			"smc #0\n" :
			"+r"(smc_id) :
			"r"(arg0), "r"(arg1), "r"(arg2), "r"(arg3)
			);
	} while (0);
	return smc_id;
}

static int tc_s4_transfer_data(char *kernel_mem_addr, uint32_t crypt_op)
{
	uint32_t index = 0;
	uint32_t copied_size = 0;

	while (copied_size < HISI_RESERVED_SECOS_PHYMEM_SIZE) {
		if (crypt_op == TSP_S4_DECRYPT_AND_COPY) {
			if (memcpy_s(g_s4_buffer_vaddr, g_s4_buffer_size,
				kernel_mem_addr + copied_size,
				g_s4_buffer_size) != EOK) {
				tloge("mem copy for decrypt failed\n");
				return -EFAULT;
			}
		}

		if (tc_s4_crypto_and_copy(crypt_op, g_s4_buffer_paddr,
			HISI_RESERVED_SECOS_PHYMEM_BASE + copied_size,
			g_s4_buffer_size, index) != 0) {
			tloge("crypto and copy failed\n");
			return -EFAULT;
		}

		if (crypt_op == TSP_S4_ENCRYPT_AND_COPY) {
			if (memcpy_s(kernel_mem_addr + copied_size,
				g_s4_buffer_size, g_s4_buffer_vaddr,
				g_s4_buffer_size) != EOK) {
				tloge("mem copy for encrypt failed\n");
				return -EFAULT;
			}
		}

		copied_size += g_s4_buffer_size;
		index++;
	}

	return 0;
}

static int tc_s4_pm_ops(struct device *dev, uint32_t power_op,
	uint32_t crypt_op)
{
	char *kernel_mem_addr = NULL;
	int ret;

	if (power_op == TSP_S4_SUSPEND) {
		ret = tc_s4_alloc_crypto_buffer(dev, &kernel_mem_addr);
		if (ret) {
			tloge("alloc s4 encrypt mem failed with ret %d\n", ret);
			return ret;
		}
		g_s4_kernel_mem_addr = kernel_mem_addr;
	} else {
		kernel_mem_addr = g_s4_kernel_mem_addr;
	}

	isb();
	wmb();

	/* notify TEEOS to suspend all pm driver */
	if (power_op == TSP_S4_SUSPEND) {
		if (tc_s4_suspend_or_resume(power_op) != 0) {
			ret = -EFAULT;
			goto free_resource;
		}
	}

	ret = tc_s4_transfer_data(kernel_mem_addr, crypt_op);
	if (ret)
		goto free_resource;

	/* notify TEEOS to resume all pm driver */
	if (power_op == TSP_S4_RESUME) {
		if (tc_s4_suspend_or_resume(power_op) != 0) {
			ret = -EFAULT;
			goto free_resource;
		}
		ret = 0;
		goto free_resource;
	}

	return 0;
free_resource:
	vunmap(g_s4_buffer_vaddr);
	vfree(kernel_mem_addr);
	g_s4_kernel_mem_addr = NULL;
	g_s4_buffer_paddr = 0;
	g_s4_buffer_vaddr = NULL;
	g_s4_buffer_size = 0;
	return ret;
}

int tc_s4_pm_suspend(struct device *dev)
{
	int ret;

	ret = tc_s4_pm_ops(dev, TSP_S4_SUSPEND, TSP_S4_ENCRYPT_AND_COPY);
	return ret;
}

int tc_s4_pm_resume(struct device *dev)
{
	int ret;

	ret = tc_s4_pm_ops(dev, TSP_S4_RESUME, TSP_S4_DECRYPT_AND_COPY);
	return ret;
}
