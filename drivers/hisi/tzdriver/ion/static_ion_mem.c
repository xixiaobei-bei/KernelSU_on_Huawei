/*
 * static_ion_mem.c
 *
 * memory init, register for mailbox pool.
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
#include "static_ion_mem.h"
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/mempool.h>
#include <linux/vmalloc.h>
#include "smc_smp.h"
#include "teek_ns_client.h"
#include "mailbox_mempool.h"
#include "tc_ns_log.h"
#include "declare_static_ion.h"


/* send the ion static memory to tee */
int tc_ns_register_ion_mem(void)
{
	struct tc_ns_smc_cmd smc_cmd = { {0}, 0 };
	int ret = 0;
	struct mb_cmd_pack *mb_pack = NULL;
	struct register_ion_mem_tag *memtag = NULL;

	mb_pack = mailbox_alloc_cmd_pack();
	if (!mb_pack) {
		tloge("mailbox alloc failed\n");
		return -ENOMEM;
	}
	memtag = mailbox_alloc(sizeof(*memtag), 0);
	if (!memtag) {
		mailbox_free(mb_pack);
		return -ENOMEM;
	}
	set_ion_mem_info(memtag);
	smc_cmd.global_cmd = true;
	smc_cmd.cmd_id = GLOBAL_CMD_ID_REGISTER_ION_MEM;

	mb_pack->operation.paramtypes = TEE_PARAM_TYPE_MEMREF_INPUT;
	mb_pack->operation.params[0].memref.buffer =
		virt_to_phys((void *)memtag);
	mb_pack->operation.buffer_h_addr[0] =
		virt_to_phys((void *)memtag) >> ADDR_TRANS_NUM;
	mb_pack->operation.params[0].memref.size = sizeof(*memtag);

	smc_cmd.operation_phys = virt_to_phys(&mb_pack->operation);
	smc_cmd.operation_h_phys =
		virt_to_phys(&mb_pack->operation) >> ADDR_TRANS_NUM;

	if (tc_ns_smc(&smc_cmd)) {
		ret = -EPERM;
		tloge("send ion mem info failed\n");
	}
	mailbox_free(mb_pack);
	mailbox_free(memtag);

	return ret;
}
