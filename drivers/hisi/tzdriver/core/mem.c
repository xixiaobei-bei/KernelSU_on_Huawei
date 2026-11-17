/*
 * mem.c
 *
 * memory operation for gp sharedmem.
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include "mem.h"
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/freezer.h>
#include <linux/module.h>
#include <linux/mempool.h>
#include <linux/vmalloc.h>
#include <linux/of_reserved_mem.h>
#include <securec.h>
#include "smc_smp.h"
#include "tc_ns_client.h"
#include "teek_ns_client.h"
#include "agent.h"
#include "tc_ns_log.h"
#include "mailbox_mempool.h"

void tc_mem_free(struct tc_ns_shared_mem *shared_mem)
{
	if (!shared_mem)
		return;

	if (shared_mem->kernel_addr) {
		vfree(shared_mem->kernel_addr);
		shared_mem->kernel_addr = NULL;
	}
	kfree(shared_mem);
}

struct tc_ns_shared_mem *tc_mem_allocate(size_t len)
{
	struct tc_ns_shared_mem *shared_mem = NULL;
	void *addr = NULL;

	shared_mem = kmalloc(sizeof(*shared_mem), GFP_KERNEL | __GFP_ZERO);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)shared_mem)) {
		tloge("shared_mem kmalloc failed\n");
		return ERR_PTR(-ENOMEM);
	}

	len = ALIGN(len, SZ_4K);
	if (len > MAILBOX_POOL_SIZE) {
		tloge("alloc sharemem size %zu is too large\n", len);
		kfree(shared_mem);
		return ERR_PTR(-EINVAL);
	}
	addr = vmalloc_user(len);
	if (!addr) {
		tloge("alloc maibox failed\n");
		kfree(shared_mem);
		return ERR_PTR(-ENOMEM);
	}
	shared_mem->kernel_addr = addr;
	shared_mem->len = len;
	shared_mem->user_addr = NULL;
	shared_mem->user_addr_ca = NULL;
	atomic_set(&shared_mem->usage, 0);
	return shared_mem;
}