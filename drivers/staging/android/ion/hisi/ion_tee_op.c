/* Copyright (c) Hisilicon Technologies Co., Ltd. 2001-2019. All rights reserved.
 * FileName: ion_tee_op.c
 * Description: This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;
 * either version 2 of the License,
 * or (at your option) any later version.
 */

#define pr_fmt(fmt) "secsg: " fmt

#include <linux/err.h>
#include <linux/sizes.h>
#include <linux/hisi/hisi_ion.h>
#include <teek_client_api.h>
#include <teek_client_id.h>
#include <teek_client_constants.h>

#include "ion.h"
#include "hisi_ion_priv.h"
#include "ion_tee_op.h"

/*uuid to TA: f8028dca-aba0-11e6-80f5-76304dec7eb7*/
#define UUID_TEEOS_TZMP2_IonMemoryManagement \
{ \
	0xf8028dca,\
	0xaba0,\
	0x11e6,\
	{ \
		0x80, 0xf5, 0x76, 0x30, 0x4d, 0xec, 0x7e, 0xb7 \
	} \
}

int secmem_tee_init(TEEC_Context *context, TEEC_Session *session)
{
	u32 root_id = 2000;
	const char *package_name = "sec_mem";
	TEEC_UUID svc_id = UUID_TEEOS_TZMP2_IonMemoryManagement;
	TEEC_Operation op = {0};
	TEEC_Result result;
	u32 origin = 0;

	if (!context || !session) {
		pr_err("Invalid context or session\n");
		goto cleanup_1;
	}

	/* initialize TEE environment */
	result = TEEK_InitializeContext(NULL, context);
	if (result != TEEC_SUCCESS) {
		pr_err("InitializeContext failed, ReturnCode=0x%x\n", result);
		goto cleanup_1;
	} else {
		sec_debug("InitializeContext success\n");
	}
	/* operation params create  */
	op.started = 1;
	op.cancel_flag = 0;
	/*open session*/
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE,
			    TEEC_NONE,
			    TEEC_MEMREF_TEMP_INPUT,
			    TEEC_MEMREF_TEMP_INPUT);

	op.params[2].tmpref.buffer = (void *)&root_id;
	op.params[2].tmpref.size = sizeof(root_id);
	op.params[3].tmpref.buffer = (void *)package_name;
	op.params[3].tmpref.size = (size_t)(strlen(package_name) + 1);

	result = TEEK_OpenSession(context, session, &svc_id,
				  TEEC_LOGIN_IDENTIFY, NULL, &op, &origin);
	if (result != TEEC_SUCCESS) {
		pr_err("OpenSession fail, RC=0x%x, RO=0x%x\n", result, origin);
		goto cleanup_2;
	} else {
		sec_debug("OpenSession success\n");
	}

	return 0;
cleanup_2:
	TEEK_FinalizeContext(context);
cleanup_1:
	return -EINVAL;
}

static void check_tee_return(u32 cmd, struct mem_chunk_list *mcl,
				TEEC_Operation op)
{
	if (cmd == ION_SEC_CMD_ALLOC) {
		mcl->buff_id = op.params[1].value.b;
		sec_debug("TEE return secbuf id 0x%x\n", mcl->buff_id);
	}

	if (cmd == ION_SEC_CMD_MAP_IOMMU) {
		mcl->va = op.params[1].value.b;
		sec_debug("TEE return iova 0x%x\n", mcl->va);
	}
}

int secmem_tee_exec_cmd(TEEC_Session *session,
		       struct mem_chunk_list *mcl, u32 cmd)
{
	TEEC_Result result;
	TEEC_Operation op = {0};
	u32 protect_id = SEC_TASK_MAX;
	u32 origin = 0;

	if (!session || !mcl)
		return -EINVAL;

	protect_id = mcl->protect_id;

	op.started = 1;
	op.cancel_flag = 0;
	op.params[0].value.a = cmd;
	op.params[0].value.b = protect_id;

	switch (cmd) {
	case ION_SEC_CMD_ALLOC:
		op.paramTypes = TEEC_PARAM_TYPES(
			TEEC_VALUE_INPUT,
			TEEC_VALUE_INOUT,
			TEEC_MEMREF_TEMP_INPUT,
			TEEC_NONE);
		op.params[1].value.a = mcl->nents;
		/* op.params[1].value.b receive the return value*/
		/* number of list in CMD buffer alloc/table set/table clean*/
		op.params[2].tmpref.buffer = mcl->phys_addr;
		op.params[2].tmpref.size =
			mcl->nents * sizeof(struct tz_pageinfo);
		break;
	case ION_SEC_CMD_MAP_IOMMU:
		op.paramTypes = TEEC_PARAM_TYPES(
			TEEC_VALUE_INPUT,
			TEEC_VALUE_INOUT,
			TEEC_NONE,
			TEEC_NONE);
		op.params[1].value.a = mcl->buff_id;
		op.params[1].value.b = mcl->size;
		break;
	case ION_SEC_CMD_FREE:
	case ION_SEC_CMD_UNMAP_IOMMU:
		op.paramTypes = TEEC_PARAM_TYPES(
			TEEC_VALUE_INPUT,
			TEEC_VALUE_INPUT,
			TEEC_NONE,
			TEEC_NONE);
		op.params[1].value.a = mcl->buff_id;
		break;
	case ION_SEC_CMD_TABLE_SET:
	case ION_SEC_CMD_TABLE_CLEAN:
		op.paramTypes = TEEC_PARAM_TYPES(
			TEEC_VALUE_INPUT,
			TEEC_VALUE_INPUT,
			TEEC_MEMREF_TEMP_INPUT,
			TEEC_NONE);
		op.params[1].value.a = mcl->nents;
		op.params[2].tmpref.buffer = mcl->phys_addr;
		op.params[2].tmpref.size =
			mcl->nents * sizeof(struct tz_pageinfo);
		break;
#ifdef CONFIG_SECMEM_TEST
	case ION_SEC_CMD_TEST:
		op.paramTypes = TEEC_PARAM_TYPES(
			TEEC_VALUE_INPUT,
			TEEC_VALUE_INPUT,
			TEEC_MEMREF_TEMP_INPUT,
			TEEC_NONE);
		op.params[1].value.a = mcl->buff_id;
		op.params[1].value.b = mcl->size;
		op.params[2].tmpref.buffer = mcl->phys_addr;
		op.params[2].tmpref.size =
			mcl->nents * sizeof(struct tz_pageinfo);
		break;
#endif
	default:
		pr_err("Invalid cmd\n");
		return -EINVAL;
	}

	result = TEEK_InvokeCommand(session, SECBOOT_CMD_ID_MEM_ALLOCATE,
				    &op, &origin);
	if (result != TEEC_SUCCESS) {
		pr_err("Invoke CMD fail, RC=0x%x, RO=0x%x\n", result, origin);
		return -EFAULT;
	}

	sec_debug("Exec TEE CMD success.\n");
	check_tee_return(cmd, mcl, op);

	return 0;
}

void secmem_tee_destroy(TEEC_Context *context, TEEC_Session *session)
{
	if (!context || !session) {
		pr_err("Invalid context or session\n");
		return;
	}

	TEEK_CloseSession(session);
	TEEK_FinalizeContext(context);
	sec_debug("TA closed !\n");
}
