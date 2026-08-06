/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 * Description: Memory init, register for mailbox pool.
 * Author: qiqingchao  q00XXXXXX
 * Create: 2016-06-21
 */
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

#include "mem.h"
#include "smc.h"
#include "tc_ns_client.h"
#include "teek_ns_client.h"
#include "agent.h"
#include "securec.h"
#include "tc_ns_log.h"
#include "mailbox_mempool.h"

void tc_mem_free(tc_ns_shared_mem *shared_mem)
{
	if (shared_mem == NULL)
		return;

	if (shared_mem->from_mailbox) {
		if (shared_mem->kernel_addr != NULL)
			mailbox_free(shared_mem->kernel_addr);
	} else {
		if (shared_mem->kernel_addr != NULL)
			vfree(shared_mem->kernel_addr);
	}

	shared_mem->kernel_addr = NULL;
	kfree(shared_mem);
	shared_mem = NULL;
}

tc_ns_shared_mem *tc_mem_allocate(size_t len, bool from_mailbox)
{
	tc_ns_shared_mem *shared_mem = NULL;
	void *addr = NULL;

	shared_mem = kmalloc(sizeof(*shared_mem), GFP_KERNEL | __GFP_ZERO);
	if (shared_mem == NULL) {
		tloge("shared_mem kmalloc failed\n");
		return ERR_PTR(-ENOMEM);
	}
	if (from_mailbox) {
		addr = mailbox_alloc(len, MB_FLAG_ZERO);
	} else {
		len = ALIGN(len, SZ_4K);
		if (len > MAILBOX_POOL_SIZE) {
			tloge("alloc sharemem size(%zu) is too large\n", len);
			kfree(shared_mem);
			return ERR_PTR(-EINVAL);
		}
		addr = vmalloc_user(len);
	}

	if (addr == NULL) {
		tloge("alloc maibox failed\n");
		kfree(shared_mem);
		return ERR_PTR(-ENOMEM);
	}
	shared_mem->from_mailbox = from_mailbox;
	shared_mem->kernel_addr = addr;
	shared_mem->len = len;
	shared_mem->user_addr = NULL;
	shared_mem->user_addr_ca = NULL;
	atomic_set(&shared_mem->usage, 0);
	return shared_mem;
}

static u64 g_ion_mem_addr;
static u64 g_ion_mem_size;

static int supersonic_reserve_tee_mem(struct reserved_mem *rmem)
{
	if (rmem != NULL) {
		g_ion_mem_addr = rmem->base;
		g_ion_mem_size = rmem->size;
	} else {
		tloge("rmem is NULL\n");
	}

	return 0;
}
RESERVEDMEM_OF_DECLARE(supersonic, "hisi-supersonic",
	supersonic_reserve_tee_mem);

static u64 g_secfacedetect_mem_addr;
static u64 g_secfacedetect_mem_size;
static int secfacedetect_reserve_tee_mem(struct reserved_mem *rmem)
{
	if (rmem != NULL) {
		g_secfacedetect_mem_addr = rmem->base;
		g_secfacedetect_mem_size = rmem->size;
	} else {
		tloge("secfacedetect_reserve_tee_mem mem is NULL\n");
	}
	return 0;
}
RESERVEDMEM_OF_DECLARE(secfacedetect, "hisi-secfacedetect",
	secfacedetect_reserve_tee_mem);

static u64 g_voiceid_addr;
static u64 g_voiceid_size;
static int voiceid_reserve_tee_mem(struct reserved_mem *rmem)
{
	if (rmem != NULL) {
		g_voiceid_addr = rmem->base;
		g_voiceid_size = rmem->size;
	} else {
		tloge("voiceid_reserve_tee_mem  mem is NULL.\n");
	}
	return 0;
}
RESERVEDMEM_OF_DECLARE(voiceid, "hisi-voiceid",
	voiceid_reserve_tee_mem);


#define ION_MEM_MAX_SIZE 5
struct register_ion_mem_tag {
	uint32_t size;
	uint64_t memaddr[ION_MEM_MAX_SIZE];
	uint32_t memsize[ION_MEM_MAX_SIZE];
};
/* send the ion static memory to tee*/
int tc_ns_register_ion_mem(void)
{
	tc_ns_smc_cmd smc_cmd = {0};
	int ret = 0;
	struct mb_cmd_pack *mb_pack = NULL;
	struct register_ion_mem_tag *memtag = NULL;
	uint32_t pos = 0;

	tloge("ion mem static reserved for tee face=%d,finger=%d,voiceid=%d\n",
		(uint32_t)g_secfacedetect_mem_size, (uint32_t)g_ion_mem_size,
		(uint32_t)g_voiceid_size);
	mb_pack = mailbox_alloc_cmd_pack();
	if (mb_pack == NULL) {
		tloge("mailbox alloc failed\n");
		return -ENOMEM;
	}
	memtag = mailbox_alloc(sizeof(*memtag), 0);
	if (memtag == NULL) {
		mailbox_free(mb_pack);
		return -ENOMEM;
	}
	if (g_ion_mem_addr != (u64)0 && g_ion_mem_size  != (u64)0) {
		memtag->memaddr[pos] = g_ion_mem_addr;
		memtag->memsize[pos] = g_ion_mem_size;
		pos++;
	}
	if (g_secfacedetect_mem_addr != (u64)0 &&
		g_secfacedetect_mem_size != (u64)0) {
		memtag->memaddr[pos] = g_secfacedetect_mem_addr;
		memtag->memsize[pos] = g_secfacedetect_mem_size;
		pos++;
	}
	if (g_voiceid_addr != (u64)0 && g_voiceid_size != (u64)0) {
		memtag->memaddr[pos] = g_voiceid_addr;
		memtag->memsize[pos] = g_voiceid_size;
		pos++;
	}
	memtag->size = pos;
	mb_pack->uuid[0] = 1;
	smc_cmd.uuid_phys = virt_to_phys(mb_pack->uuid);
	smc_cmd.uuid_h_phys = virt_to_phys(mb_pack->uuid) >> 32;
	smc_cmd.cmd_id = GLOBAL_CMD_ID_REGISTER_ION_MEM;

	mb_pack->operation.paramtypes = TEE_PARAM_TYPE_MEMREF_INPUT;
	mb_pack->operation.params[0].memref.buffer = virt_to_phys((void *)memtag);
	mb_pack->operation.buffer_h_addr[0] = virt_to_phys((void *)memtag) >> 32;
	mb_pack->operation.params[0].memref.size =  sizeof(*memtag);

	smc_cmd.operation_phys = virt_to_phys(&mb_pack->operation);
	smc_cmd.operation_h_phys = virt_to_phys(&mb_pack->operation) >> 32;

	ret = tc_ns_smc(&smc_cmd);
	mailbox_free(mb_pack);
	mailbox_free(memtag);
	if (ret)
		tloge("Send ion mem info failed.\n");

	return ret;
}

int tc_mem_init(void)
{
	int ret;

	tlogi("tc_mem_init\n");

	ret = mailbox_mempool_init();
	if (ret) {
		tloge("tz mailbox init failed\n");
		return -ENOMEM;
	}

	return 0;
}

void tc_mem_destroy(void)
{
	tlogi("tc_client exit\n");

	mailbox_mempool_destroy();
}
