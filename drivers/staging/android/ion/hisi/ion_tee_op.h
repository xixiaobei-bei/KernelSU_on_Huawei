/* Copyright (c) Hisilicon Technologies Co., Ltd. 2001-2019. All rights reserved.
 * FileName: ion_tee_op.h
 * Description: This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;
 * either version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef _ION_TEE_OP_H
#define _ION_TEE_OP_H

#include <teek_client_api.h>
#include <teek_client_id.h>
#include <teek_client_constants.h>

enum SEC_Task {
	SEC_TASK_DRM = 0x0,
	SEC_TASK_SEC,
	SEC_TASK_MAX,
};

#define SECBOOT_CMD_ID_MEM_ALLOCATE 0x1

#ifdef CONFIG_HISI_ION_SECSG_DEBUG
#define sec_debug(fmt, ...) \
	pr_info(fmt, ##__VA_ARGS__)
#else
#define sec_debug(fmt, ...)
#endif

struct mem_chunk_list {
	u32 protect_id;
	union {
		u32 nents;
		u32 buff_id;
	};
	u32 va;
	void *phys_addr;  /* Must be the start addr of struct tz_pageinfo */
	u32 size;
};

struct tz_pageinfo {
	u64 addr;
	u32 nr_pages;
} __aligned(8);

int secmem_tee_init(TEEC_Context *context, TEEC_Session *session);

int secmem_tee_exec_cmd(TEEC_Session *session,
		       struct mem_chunk_list *mcl, u32 cmd);

void secmem_tee_destroy(TEEC_Context *context, TEEC_Session *session);

void seccg_pre_alloc_wk_func(struct work_struct *work);
#endif
