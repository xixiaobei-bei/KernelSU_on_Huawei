/*
 * dynamic_mem.c
 *
 * dynamic Ion memory allocation and free.
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
#include "dynamic_ion_mem.h"
#include <stdarg.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/ion.h>
#include <linux/mm.h>
#include <linux/cma.h>
#include <linux/hisi/hisi_ion.h>
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include "tc_ns_log.h"
#include "tc_ns_client.h"
#include "smc_smp.h"
#include "teek_client_constants.h"
#include "mailbox_mempool.h"
#include "dynamic_ion_uuid.h"

static DEFINE_MUTEX(dynamic_mem_lock);
struct dynamic_mem_list {
	struct list_head list;
};

static const struct dynamic_mem_config g_dyn_mem_config[] = {
	{ TEE_SECIDENTIFICATION1, SEC_EID },
	{ TEE_SECIDENTIFICATION3, SEC_EID },
	{ TEE_SERVICE_HIAI, SEC_HIAI },
	{ TEE_SERVICE_HIAI_TINY, SEC_HIAI },
};

static struct dynamic_mem_list g_dynamic_mem_list;
static const uint32_t g_dyn_mem_config_num = ARRAY_SIZE(g_dyn_mem_config);

static int send_loadapp_ion(void)
{
	struct tc_ns_smc_cmd smc_cmd = { {0}, 0 };
	int ret = 0;
	struct mb_cmd_pack *mb_pack = NULL;

	mb_pack = mailbox_alloc_cmd_pack();
	if (!mb_pack) {
		tloge("alloc cmd pack failed\n");
		return -ENOMEM;
	}
	smc_cmd.global_cmd = true;
	smc_cmd.cmd_id = GLOBAL_CMD_ID_LOAD_SECURE_APP_ION;
	mb_pack->operation.paramtypes = teec_param_types(TEE_PARAM_TYPE_NONE,
		TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
	smc_cmd.operation_phys =
		(unsigned int)virt_to_phys(&mb_pack->operation);
	smc_cmd.operation_h_phys =
		virt_to_phys(&mb_pack->operation) >> ADDR_TRANS_NUM;

	if (tc_ns_smc(&smc_cmd)) {
		ret = -EPERM;
		tlogd("send loadapp ion failed\n");
	}
	mailbox_free(mb_pack);
	return ret;
}

static int get_ion_sglist(struct dynamic_mem_item *mem_item)
{
	struct sglist *tmp_sglist = NULL;
	struct scatterlist *sg = NULL;
	struct page *page = NULL;
	uint32_t sglist_size;
	uint32_t i = 0;
	struct sg_table *ion_sg_table = mem_item->memory.dyn_sg_table;

	if (!ion_sg_table)
		return -EINVAL;

	if (ion_sg_table->nents <= 0 || ion_sg_table->nents > MAX_ION_NENTS)
		return -EINVAL;

	for_each_sg(ion_sg_table->sgl, sg, ion_sg_table->nents, i) {
		if (!sg) {
			tloge("an error sg when get ion sglist\n");
			return -EINVAL;
		}
	}

	sglist_size = sizeof(struct ion_page_info) * ion_sg_table->nents +
		sizeof(*tmp_sglist);
	tmp_sglist = (struct sglist *)mailbox_alloc(sglist_size, MB_FLAG_ZERO);
	if (!tmp_sglist) {
		tloge("mailbox alloc failed\n");
		return -ENOMEM;
	}
	tmp_sglist->sglist_size = (uint64_t)sglist_size;
	tmp_sglist->ion_size = (uint64_t)mem_item->size;
	tmp_sglist->info_length = (uint64_t)ion_sg_table->nents;
	for_each_sg(ion_sg_table->sgl, sg, ion_sg_table->nents, i) {
		page = sg_page(sg);
		tmp_sglist->page_info[i].phys_addr = page_to_phys(page);
		tmp_sglist->page_info[i].npages = sg->length / PAGE_SIZE;
	}
	mem_item->memory.ion_phys_addr = virt_to_phys((void *)tmp_sglist);
	mem_item->memory.len = sglist_size;
	return 0;
}

static int register_to_tee(struct dynamic_mem_item *mem_item)
{
	struct tc_ns_smc_cmd smc_cmd = { {0}, 0 };
	int ret;
	struct mb_cmd_pack *mb_pack = NULL;

	if (!mem_item) {
		tloge("mem_item is null\n");
		return -EINVAL;
	}

	ret = get_ion_sglist(mem_item);
	if (ret)
		return ret;

	mb_pack = mailbox_alloc_cmd_pack();
	if (!mb_pack) {
		mailbox_free(phys_to_virt(mem_item->memory.ion_phys_addr));
		tloge("alloc cmd pack failed\n");
		return -ENOMEM;
	}
	smc_cmd.global_cmd = true;
	smc_cmd.cmd_id = GLOBAL_CMD_ID_ADD_DYNAMIC_ION;
	mb_pack->operation.paramtypes = teec_param_types(
		TEE_PARAM_TYPE_ION_SGLIST_INPUT,
		TEE_PARAM_TYPE_VALUE_INPUT,
		TEE_PARAM_TYPE_VALUE_INPUT,
		TEE_PARAM_TYPE_NONE);

	mb_pack->operation.params[0].memref.size =
		(uint32_t)mem_item->memory.len;
	mb_pack->operation.params[0].memref.buffer =
		(uint32_t)(mem_item->memory.ion_phys_addr & 0xFFFFFFFF);
	mb_pack->operation.buffer_h_addr[0] =
		(uint32_t)(mem_item->memory.ion_phys_addr >> ADDR_TRANS_NUM);
	mb_pack->operation.params[1].value.a =
		(uint32_t)mem_item->size;
	mb_pack->operation.params[2].value.a = mem_item->configid;
	smc_cmd.operation_phys =
		(unsigned int)virt_to_phys(&mb_pack->operation);
	smc_cmd.operation_h_phys =
		virt_to_phys(&mb_pack->operation) >> ADDR_TRANS_NUM;

	if (tc_ns_smc(&smc_cmd)) {
		ret = -EPERM;
		tlogd("send loadapp ion failed\n");
	}
	mailbox_free(phys_to_virt(mem_item->memory.ion_phys_addr));
	mailbox_free(mb_pack);
	return ret;
}

static int unregister_from_tee(struct dynamic_mem_item *mem_item)
{
	struct tc_ns_smc_cmd smc_cmd = { {0}, 0 };
	int ret;
	struct mb_cmd_pack *mb_pack = NULL;

	if (!mem_item) {
		tloge("mem_item is null\n");
		return -EINVAL;
	}

	ret = get_ion_sglist(mem_item);
	if (ret)
		return ret;

	mb_pack = mailbox_alloc_cmd_pack();
	if (!mb_pack) {
		mailbox_free(phys_to_virt(mem_item->memory.ion_phys_addr));
		tloge("alloc cmd pack failed\n");
		return -ENOMEM;
	}

	smc_cmd.global_cmd = true;
	smc_cmd.cmd_id = GLOBAL_CMD_ID_DEL_DYNAMIC_ION;
	mb_pack->operation.paramtypes = teec_param_types(
		TEE_PARAM_TYPE_ION_SGLIST_INPUT,
		TEE_PARAM_TYPE_VALUE_INPUT,
		TEE_PARAM_TYPE_VALUE_INPUT,
		TEE_PARAM_TYPE_NONE);

	mb_pack->operation.params[0].memref.size =
		(uint32_t)mem_item->memory.len;
	mb_pack->operation.params[0].memref.buffer =
		(uint32_t)(mem_item->memory.ion_phys_addr & 0xFFFFFFFF);
	mb_pack->operation.buffer_h_addr[0] =
		(uint32_t)(mem_item->memory.ion_phys_addr >> ADDR_TRANS_NUM);
	mb_pack->operation.params[1].value.a =
		(uint32_t)mem_item->size;
	mb_pack->operation.params[2].value.a = mem_item->configid;
	smc_cmd.operation_phys =
		(unsigned int)virt_to_phys(&mb_pack->operation);
	smc_cmd.operation_h_phys =
		virt_to_phys(&mb_pack->operation) >> ADDR_TRANS_NUM;
	if (tc_ns_smc(&smc_cmd)) {
		ret = -EPERM;
		tlogd("unregister from tee failed\n");
	}
	mailbox_free(phys_to_virt(mem_item->memory.ion_phys_addr));
	mailbox_free(mb_pack);

	return ret;
}

static struct dynamic_mem_item *find_memitem_by_configid_locked(
	uint32_t configid)
{
	struct dynamic_mem_item *item = NULL;

	list_for_each_entry(item, &g_dynamic_mem_list.list, head) {
		if (item->configid == configid)
			return item;
	}

	return NULL;
}

static struct dynamic_mem_item *find_memitem_by_uuid_locked(
	const struct tc_uuid *uuid)
{
	struct dynamic_mem_item *item = NULL;

	list_for_each_entry(item, &g_dynamic_mem_list.list, head) {
		if (!memcmp(&item->uuid, uuid, sizeof(*uuid)))
			return item;
	}

	return NULL;
}

#define BLOCK_64KB_SIZE   (64 * 1024) /* 64KB */
#define BLOCK_64KB_MASK   0xFFFFFFFFFFFF0000
/* size should be aligned with 64KB */
#define BLOCK_64KB_SIZE_MASK (BLOCK_64KB_SIZE - 1)
static int alloc_from_hisi(struct dynamic_mem_item *mem_item)
{
	struct sg_table *ion_sg_table = NULL;

	if (mem_item->size + BLOCK_64KB_SIZE_MASK < mem_item->size) {
		tloge("ion size is error, size = %x\n", mem_item->size);
		return -EINVAL;
	}
	mem_item->memory.len = (mem_item->size + BLOCK_64KB_SIZE_MASK) &
		BLOCK_64KB_MASK;

	ion_sg_table = mm_secmem_alloc(mem_item->ddr_sec_region,
		mem_item->memory.len);
	if (!ion_sg_table) {
		tloge("failed to get ion page, configid=%d\n",
			mem_item->configid);
		return -ENOMEM;
	}
	mem_item->memory.dyn_sg_table = ion_sg_table;
	return 0;
}

static void free_to_hisi(struct dynamic_mem_item *mem_item)
{
	if (!mem_item->memory.dyn_sg_table) {
		tloge("ion_phys_addr is NULL\n");
		return;
	}
	mm_secmem_free(mem_item->ddr_sec_region,
		mem_item->memory.dyn_sg_table);
	mem_item->memory.dyn_sg_table = NULL;
	return;
}

int init_dynamic_mem(void)
{
	INIT_LIST_HEAD(&(g_dynamic_mem_list.list));
	return 0;
}

static int32_t find_ddr_sec_region_by_uuid(const struct tc_uuid *uuid,
	uint32_t *ddr_sec_region)
{
	uint32_t i;

	for (i = 0; i < g_dyn_mem_config_num; i++) {
		if (!memcmp(&(g_dyn_mem_config[i].uuid), uuid,
			sizeof(*uuid))) {
			*ddr_sec_region = g_dyn_mem_config[i].ddr_sec_region;
			return 0;
		}
	}
	return -EINVAL;
}

static struct dynamic_mem_item *alloc_dyn_mem_item(uint32_t configid,
	uint32_t cafd, const struct tc_uuid *uuid, uint32_t size)
{
	uint32_t ddr_sec_region;
	struct dynamic_mem_item *mem_item = NULL;
	int32_t result;

	result = find_ddr_sec_region_by_uuid(uuid, &ddr_sec_region);
	if (result) {
		tloge("find ddr sec region failed\n");
		return NULL;
	}

	mem_item = kzalloc(sizeof(*mem_item), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)mem_item)) {
		tloge("alloc mem item failed\n");
		return NULL;
	}

	mem_item->ddr_sec_region = ddr_sec_region;
	mem_item->configid = configid;
	mem_item->size = size;
	mem_item->cafd = cafd;
	result = memcpy_s(&mem_item->uuid, sizeof(mem_item->uuid), uuid,
		sizeof(*uuid));
	if (result != EOK) {
		tloge("memcpy uuid failed\n");
		kfree(mem_item);
		return NULL;
	}
	return mem_item;
}

static int trans_configid2memid(uint32_t configid, uint32_t cafd,
	const struct tc_uuid *uuid, uint32_t size)
{
	int result;

	if (!uuid)
		return -EINVAL;

	/* if config id is memid map,and can reuse */
	mutex_lock(&dynamic_mem_lock);
	do {
		struct dynamic_mem_item *mem_item =
			find_memitem_by_configid_locked(configid);
		if (mem_item) {
			result = -EINVAL;
			break;
		}

		mem_item = alloc_dyn_mem_item(configid, cafd, uuid, size);
		if (!mem_item) {
			tloge("alloc dyn mem item failed\n");
			result = -ENOMEM;
			break;
		}

		result = alloc_from_hisi(mem_item);
		if (result) {
			tloge("alloc from hisi failed ,ret = %d\n", result);
			kfree(mem_item);
			break;
		}
		/* register to tee */
		result = register_to_tee(mem_item);
		if (result) {
			tloge("register to tee failed ,result =%d\n", result);
			free_to_hisi(mem_item);
			kfree(mem_item);
			break;
		}
		list_add_tail(&mem_item->head, &g_dynamic_mem_list.list);
		tloge("log import:alloc ion from hisi configid=%d\n",
			mem_item->configid);
	} while (0);

	mutex_unlock(&dynamic_mem_lock);
	return result;
}

static void release_configid_mem_locked(uint32_t configid)
{
	int result;

	/* if config id is memid map,and can reuse */
	do {
		struct dynamic_mem_item *mem_item =
			find_memitem_by_configid_locked(configid);
		if (!mem_item) {
			tloge("fail to find memitem by configid\n");
			break;
		}
		/* register to tee */
		result = unregister_from_tee(mem_item);
		if (result) {
			tloge("unregister_from_tee configid=%d,result=%d\n",
				mem_item->configid, result);
			break;
		}
		free_to_hisi(mem_item);
		list_del(&mem_item->head);
		kfree(mem_item);
		tloge("log import: free ion to hisi\n");
	} while (0);

	return;
}

void release_configid_mem(uint32_t configid)
{
	mutex_lock(&dynamic_mem_lock);
	release_configid_mem_locked(configid);
	mutex_unlock(&dynamic_mem_lock);
	return;
}

int load_app_use_configid(uint32_t configid, uint32_t cafd,
	const struct tc_uuid *uuid, uint32_t size)
{
	int result;

	if (!uuid)
		return -EINVAL;

	result = trans_configid2memid(configid, cafd, uuid, size);
	if (result) {
		tloge("trans_configid2memid faild ret=%d\n", result);
		return result;
	}

	result = send_loadapp_ion();
	if (result) {
		release_configid_mem(configid);
		tloge("send_loadapp_ion failed ret=%d\n", result);
	}
	return result;
}

void kill_ion_by_uuid(const struct tc_uuid *uuid)
{
	if (!uuid) {
		tloge("uuid is null\n");
		return;
	}
	mutex_lock(&dynamic_mem_lock);
	do {
		struct dynamic_mem_item *mem_item =
			find_memitem_by_uuid_locked(uuid);
		if (!mem_item)
			break;
		tlogd("kill ION by UUID\n");
		release_configid_mem_locked(mem_item->configid);
	} while (0);
	mutex_unlock(&dynamic_mem_lock);
}

void kill_ion_by_cafd(unsigned int cafd)
{
	struct dynamic_mem_item *item = NULL;
	struct dynamic_mem_item *temp = NULL;

	tlogd("kill_ion_by_cafd:\n");
	mutex_lock(&dynamic_mem_lock);
	list_for_each_entry_safe(item, temp, &g_dynamic_mem_list.list, head) {
		if (item->cafd == cafd)
			release_configid_mem_locked(item->configid);
	}
	mutex_unlock(&dynamic_mem_lock);
}
