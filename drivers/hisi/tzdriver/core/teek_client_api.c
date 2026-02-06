/*
 * teek_client_api.c
 *
 * function definition for libteec interface for kernel CA.
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
#include "teek_client_api.h"
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <asm/cacheflush.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/kernel.h>
#include <securec.h>
#include "teek_client_id.h"
#include "tc_ns_log.h"
#include "tc_ns_client.h"
#include "gp_ops.h"
#include "session_manager.h"
#include "tc_client_driver.h"

static void encode_for_part_mem(struct tc_ns_client_context *context,
	const struct teec_operation *oper, uint32_t idex, uint32_t *param_type)
{
	uint32_t diff = (uint32_t)TEEC_MEMREF_PARTIAL_INPUT -
		(uint32_t)TEEC_MEMREF_TEMP_INPUT;

	if (idex >= TEE_PARAM_NUM)
		return;

	if (param_type[idex] == TEEC_MEMREF_WHOLE) {
		context->params[idex].memref.offset = 0;
		context->params[idex].memref.size_addr =
			(__u64)(uintptr_t)
			(&(oper->params[idex].memref.parent->size));
	} else {
		context->params[idex].memref.offset =
			oper->params[idex].memref.offset;
		context->params[idex].memref.size_addr =
			(__u64)(uintptr_t)
			(&(oper->params[idex].memref.size));
	}
	if (oper->params[idex].memref.parent->is_allocated) {
		context->params[idex].memref.buffer =
			(__u64)(uintptr_t)
			oper->params[idex].memref.parent->buffer;
	} else {
		context->params[idex].memref.buffer =
			(__u64)(uintptr_t)
			oper->params[idex].memref.parent->buffer +
			oper->params[idex].memref.offset;
		context->params[idex].memref.offset = 0;
	}

	/* translate the paramType to know the driver */
	if (param_type[idex] == TEEC_MEMREF_WHOLE) {
		switch (oper->params[idex].memref.parent->flags) {
		case TEEC_MEM_INPUT:
			param_type[idex] = TEEC_MEMREF_PARTIAL_INPUT;
			break;
		case TEEC_MEM_OUTPUT:
			param_type[idex] = TEEC_MEMREF_PARTIAL_OUTPUT;
			break;
		case TEEC_MEM_INOUT:
			param_type[idex] = TEEC_MEMREF_PARTIAL_INOUT;
			break;
		default:
			param_type[idex] = TEEC_MEMREF_PARTIAL_INOUT;
			break;
		}
	}

	/* if not allocated, trans PARTIAL_XXX to MEMREF_TEMP_XXX */
	if (!oper->params[idex].memref.parent->is_allocated)
		param_type[idex] = param_type[idex] - diff;
}

static uint32_t proc_teek_encode(struct tc_ns_client_context *cli_context,
	const struct teec_operation *operation)
{
	uint32_t param_type[TEE_PARAM_NUM];
	uint32_t idex;

	param_type[0] =
		teec_param_type_get(operation->paramtypes, 0);
	param_type[1] =
		teec_param_type_get(operation->paramtypes, 1);
	param_type[2] =
		teec_param_type_get(operation->paramtypes, 2);
	param_type[3] =
		teec_param_type_get(operation->paramtypes, 3);
	for (idex = 0; idex < TEE_PARAM_NUM; idex++) {
		if (is_tmp_mem(param_type[idex])) {
			cli_context->params[idex].memref.buffer =
				(__u64)(uintptr_t)
				(operation->params[idex].tmpref.buffer);
			cli_context->params[idex].memref.size_addr =
				(__u64)(uintptr_t)
				(&operation->params[idex].tmpref.size);
		} else if (is_ref_mem(param_type[idex])) {
			encode_for_part_mem(cli_context, operation,
				idex, param_type);
		} else if (is_val_param(param_type[idex])) {
			cli_context->params[idex].value.a_addr =
				(__u64)(uintptr_t)
				(&(operation->params[idex].value.a));
			cli_context->params[idex].value.b_addr =
				(__u64)(uintptr_t)
				(&(operation->params[idex].value.b));
		} else if (is_ion_param(param_type[idex])) {
			cli_context->params[idex].value.a_addr =
				(__u64)(uintptr_t)
				(&(operation->params[idex].ionref.ion_share_fd));
			cli_context->params[idex].value.b_addr =
				(__u64)(uintptr_t)
				(&(operation->params[idex].ionref.ion_size));
		} else if (param_type[idex] == TEEC_NONE) {
			/* do nothing */
		} else {
			tloge("param_type[%u]=%u not correct\n", idex,
				param_type[idex]);
			return TEEC_ERROR_BAD_PARAMETERS;
		}
	}
	cli_context->param_types = teec_param_types(param_type[0],
		param_type[1], param_type[2], param_type[3]);

	return TEEC_SUCCESS;
}

static uint32_t teek_init_context(struct tc_ns_client_context *cli_context,
	struct teec_uuid service_id, uint32_t session_id, uint32_t cmd_id,
	const struct tc_ns_client_login *cli_login)
{
	uint32_t diff;

	diff = (uint32_t)TEEC_MEMREF_PARTIAL_INPUT -
		(uint32_t)TEEC_MEMREF_TEMP_INPUT;

	if (memset_s(cli_context, sizeof(*cli_context),
		0x00, sizeof(*cli_context))) {
		tloge("memset error\n");
		return TEEC_ERROR_BAD_PARAMETERS;
	}

	if (memcpy_s(cli_context->uuid, sizeof(cli_context->uuid),
		(uint8_t *)&service_id, sizeof(service_id))) {
		tloge("memcpy error\n");
		return TEEC_ERROR_BAD_PARAMETERS;
	}
	cli_context->session_id = session_id;
	cli_context->cmd_id = cmd_id;
	cli_context->returns.code = 0;
	cli_context->returns.origin = 0;
	cli_context->login.method = cli_login->method;
	cli_context->login.mdata = cli_login->mdata;

	return TEEC_SUCCESS;
}

static uint32_t teek_check_tmp_mem(
	const struct teec_tempmemory_reference *tmpref)
{
	if (!tmpref->buffer || !tmpref->size) {
		tloge("tmpref buffer is null, or size is zero\n");
		return TEEC_ERROR_BAD_PARAMETERS;
	}

	return TEEC_SUCCESS;
}

static bool is_partical_mem(uint32_t param_type)
{
	if (param_type == TEEC_MEMREF_PARTIAL_INPUT ||
		param_type == TEEC_MEMREF_PARTIAL_OUTPUT ||
		param_type == TEEC_MEMREF_PARTIAL_INOUT)
		return true;

	return false;
}

static bool is_offset_invalid(
	const struct teec_registeredmemory_reference *memref)
{
	if ((memref->offset + memref->size > memref->parent->size) ||
			(memref->offset + memref->size < memref->offset) ||
			(memref->offset + memref->size < memref->size))
		return true;

	return false;
}

static uint32_t teek_check_ref_mem(
	const struct teec_registeredmemory_reference *memref,
	uint32_t param_type)
{
	if (!memref->parent || !memref->parent->buffer) {
		tloge("parent of memref is null, or the buffer is zero\n");
		return TEEC_ERROR_BAD_PARAMETERS;
	}
	if (param_type == TEEC_MEMREF_PARTIAL_INPUT) {
		if (!(memref->parent->flags & TEEC_MEM_INPUT))
			return TEEC_ERROR_BAD_PARAMETERS;
	} else if (param_type == TEEC_MEMREF_PARTIAL_OUTPUT) {
		if (!(memref->parent->flags & TEEC_MEM_OUTPUT))
			return TEEC_ERROR_BAD_PARAMETERS;
	} else if (param_type == TEEC_MEMREF_PARTIAL_INOUT) {
		if (!(memref->parent->flags & TEEC_MEM_INPUT))
			return TEEC_ERROR_BAD_PARAMETERS;
		if (!(memref->parent->flags & TEEC_MEM_OUTPUT))
			return TEEC_ERROR_BAD_PARAMETERS;
	} else if (param_type == TEEC_MEMREF_WHOLE) {
		/* if type is TEEC_MEMREF_WHOLE, ignore it */
	} else {
		return TEEC_ERROR_BAD_PARAMETERS;
	}

	if (is_partical_mem(param_type)) {
		if (is_offset_invalid(memref)) {
			tloge("offset + size exceed the parent size\n");
			return TEEC_ERROR_BAD_PARAMETERS;
		}
	}
	return TEEC_SUCCESS;
}

/*
 * This function checks a operation is valid or not.
 */
uint32_t teek_check_operation(const struct teec_operation *operation)
{
	uint32_t param_type[TEE_PARAM_NUM] = {0};
	uint32_t idex;
	uint32_t ret = TEEC_SUCCESS;

	/*
	 * GP Support operation is NULL
	 * operation: a pointer to a Client Application initialized struct,
	 * or NULL if there is no payload to send or
	 * if the Command does not need to support cancellation.
	 */
	if (!operation)
		return TEEC_SUCCESS;

	if (!operation->started) {
		tloge("sorry, cancellation not support\n");
		return TEEC_ERROR_NOT_IMPLEMENTED;
	}

	param_type[0] =
		teec_param_type_get(operation->paramtypes, 0);
	param_type[1] =
		teec_param_type_get(operation->paramtypes, 1);
	param_type[2] =
		teec_param_type_get(operation->paramtypes, 2);
	param_type[3] =
		teec_param_type_get(operation->paramtypes, 3);
	for (idex = 0; idex < TEE_PARAM_NUM; idex++) {
		if (is_tmp_mem(param_type[idex])) {
			ret = teek_check_tmp_mem(
				&(operation->params[idex].tmpref));
			if (ret != TEEC_SUCCESS)
				break;
		} else if (is_ref_mem(param_type[idex])) {
			ret = teek_check_ref_mem(
				&(operation->params[idex].memref),
				param_type[idex]);
			if (ret != TEEC_SUCCESS)
				break;
		} else if (is_val_param(param_type[idex])) {
			/* do nothing */
		} else if (is_ion_param(param_type[idex])) {
			if (operation->params[idex].ionref.ion_share_fd < 0) {
				tloge("ion_handle is invalid!\n");
				ret = TEEC_ERROR_BAD_PARAMETERS;
				break;
			}
		} else if (param_type[idex] == TEEC_NONE) {
			/* do nothing */
		} else {
			tloge("paramType[%u]=%x is not support\n", idex,
				param_type[idex]);
			ret = TEEC_ERROR_BAD_PARAMETERS;
			break;
		}
	}

	return ret;
}

/*
 * This function check if the special agent is launched.Used For HDCP key.
 * e.g. If sfs agent is not alive, you can not do HDCP key write to SRAM.
 */
int teek_is_agent_alive(unsigned int agent_id)
{
	return is_agent_alive(agent_id);
}

/*
 * This function initializes a new TEE Context,
 * forming a connection between this Client Application
 * and the TEE identified by the string identifier name.
 */
uint32_t teek_initialize_context(const char *name,
	struct teec_context *context)
{
	int32_t ret;

	/* name current not used */
	(void)(name);
	tlogd("teek_initialize_context Started:\n");
	/* First, check parameters is valid or not */
	if (!context) {
		tloge("context is null, not correct\n");
		return TEEC_ERROR_BAD_PARAMETERS;
	}
	context->dev = NULL;
	/* Paramters right, start execution */
	ret = tc_ns_client_open((struct tc_ns_dev_file **)&context->dev,
		TEE_REQ_FROM_KERNEL_MODE);
	if (ret != TEEC_SUCCESS) {
		tloge("open device failed\n");
		return TEEC_ERROR_GENERIC;
	}
	tlogd("open device success\n");
	return TEEC_SUCCESS;
}

/*
 * This function finalizes an initialized TEE Context.
 */
void teek_finalize_context(struct teec_context *context)
{
	tlogd("teek_finalize_context started\n");
	if (!context || !context->dev) {
		tloge("context or dev is null, not correct\n");
		return;
	}

	tlogd("close device\n");
	tc_ns_client_close(context->dev);
	context->dev = NULL;
}

static bool is_oper_param_valid(const struct teec_operation *operation)
{
	uint32_t param_type[TEE_PARAM_NUM] = {0};

	param_type[3] =
		teec_param_type_get(operation->paramtypes, 3);
	param_type[2] =
		teec_param_type_get(operation->paramtypes, 2);

	if (param_type[3] != TEEC_MEMREF_TEMP_INPUT ||
		param_type[2] != TEEC_MEMREF_TEMP_INPUT) {
		tloge("invalid param type 0x%x\n", operation->paramtypes);
		return false;
	}

	if (!operation->params[3].tmpref.buffer ||
		!operation->params[2].tmpref.buffer ||
		operation->params[3].tmpref.size == 0 ||
		operation->params[2].tmpref.size == 0) {
		tloge("invalid operation params(NULL)\n");
		return false;
	}
	return true;
}

static uint32_t check_open_sess_params(struct teec_context *context,
	const struct teec_operation *operation)
{
	struct tc_ns_dev_file *dev_file = NULL;
	uint32_t teec_ret;

	if (!is_oper_param_valid(operation))
		return TEEC_ERROR_BAD_PARAMETERS;

	dev_file = (struct tc_ns_dev_file *)(context->dev);
	if (!dev_file) {
		tloge("invalid context->dev (NULL)\n");
		return TEEC_ERROR_BAD_PARAMETERS;
	}

	dev_file->pkg_name_len = operation->params[3].tmpref.size;
	if (operation->params[3].tmpref.size > MAX_PACKAGE_NAME_LEN - 1) {
		return TEEC_ERROR_BAD_PARAMETERS;
	} else {
		if (memset_s(dev_file->pkg_name, sizeof(dev_file->pkg_name),
			0, MAX_PACKAGE_NAME_LEN)) {
			tloge("memset error\n");
			return TEEC_ERROR_BAD_PARAMETERS;
		}
		if (memcpy_s(dev_file->pkg_name, sizeof(dev_file->pkg_name),
			operation->params[3].tmpref.buffer,
			operation->params[3].tmpref.size)) {
			tloge("memcpy error\n");
			return TEEC_ERROR_BAD_PARAMETERS;
		}
	}

	dev_file->pub_key_len = 0;
	dev_file->login_setup = 1;
	teec_ret = teek_check_operation(operation);
	if (teec_ret != TEEC_SUCCESS) {
		tloge("operation is invalid\n");
		return TEEC_ERROR_BAD_PARAMETERS;
	}

	return teec_ret;
}

static uint32_t open_session_and_switch_ret(struct teec_session *session,
	struct teec_context *context, const struct teec_uuid *destination,
	struct tc_ns_client_context *cli_context, uint32_t *origin)
{
	int32_t ret;
	uint32_t teec_ret;

	ret = tc_ns_open_session(context->dev, cli_context);
	if (!ret) {
		tlogd("open session success\n");
		session->session_id = cli_context->session_id;
		session->service_id = *destination;
		session->ops_cnt = 0;
		session->context = context;
		return TEEC_SUCCESS;
	} else if (ret < 0) {
		tloge("open session failed, ioctl errno = %u\n", ret);
		if (ret == -EFAULT)
			teec_ret = TEEC_ERROR_ACCESS_DENIED;
		else if (ret == -ENOMEM)
			teec_ret = TEEC_ERROR_OUT_OF_MEMORY;
		else if (ret == -EINVAL)
			teec_ret = TEEC_ERROR_BAD_PARAMETERS;
		else if (ret == -ERESTARTSYS)
			teec_ret = TEEC_CLIENT_INTR;
		else
			teec_ret = TEEC_ERROR_GENERIC;
		*origin = TEEC_ORIGIN_COMMS;
		return teec_ret;
	} else {
		tloge("open session failed, code=0x%x, origin=%u\n",
			cli_context->returns.code,
			cli_context->returns.origin);
		teec_ret = (uint32_t)cli_context->returns.code;
		*origin = cli_context->returns.origin;
	}
	return teec_ret;
}

static uint32_t proc_teek_open_session(struct teec_context *context,
	struct teec_session *session, const struct teec_uuid *destination,
	uint32_t connection_method, const void *connection_data,
	const struct teec_operation *operation, uint32_t *return_origin)
{
	uint32_t teec_ret;
	uint32_t origin = TEEC_ORIGIN_API;
	struct tc_ns_client_context cli_context;
	struct tc_ns_client_login cli_login = {0};

	tlogd("teek_open_session Started:\n");

	/* connectionData current not used */
	(void)(connection_data);
	if (return_origin)
		*return_origin = origin;

	/* First, check parameters is valid or not */
	if (!context || !operation || !destination ||
		!session || connection_method != TEEC_LOGIN_IDENTIFY) {
		tloge("invalid input params\n");
		teec_ret = TEEC_ERROR_BAD_PARAMETERS;
		goto set_ori;
	}

	cli_login.method = TEEC_LOGIN_IDENTIFY;
	teec_ret = check_open_sess_params(context, operation);
	if (teec_ret != TEEC_SUCCESS)
		goto set_ori;

	teec_ret = teek_init_context(&cli_context, *destination, 0,
		GLOBAL_CMD_ID_OPEN_SESSION, &cli_login);
	if (teec_ret != TEEC_SUCCESS) {
		tloge("init cli context failed\n");
		goto set_ori;
	}

	/* support when operation is null */
	if (operation) {
		cli_context.started = operation->cancel_flag;
		teec_ret = proc_teek_encode(&cli_context, operation);
		tlogv("cli param type %u\n", cli_context.param_types);
		if (teec_ret != TEEC_SUCCESS)
			goto set_ori;
	}

#ifdef CONFIG_AUTH_ENHANCE
	cli_context.teec_token = session->teec_token;
	cli_context.token_len = sizeof(session->teec_token);
#endif
	teec_ret = open_session_and_switch_ret(session, context,
		destination, &cli_context, &origin);
set_ori:
	if (teec_ret != TEEC_SUCCESS && return_origin != NULL)
		*return_origin = origin;

	return teec_ret;
}

#define RETRY_TIMES 5
uint32_t teek_open_session(struct teec_context *context,
	struct teec_session *session, const struct teec_uuid *destination,
	uint32_t connection_method, const void *connection_data,
	const struct teec_operation *operation, uint32_t *return_origin)
{
	int i;
	uint32_t ret;

	for (i = 0; i < RETRY_TIMES; i++) {
		ret = proc_teek_open_session(context, session,
			destination, connection_method, connection_data,
			operation, return_origin);
		if (ret != (uint32_t)TEEC_CLIENT_INTR)
			return ret;
	}
	return ret;
}

/*
 * This function closes an opened Session.
 */

static bool is_close_sess_param_valid(struct teec_session *session)
{
	tlogd("teek_close_session started\n");

	if (!session || !session->context || !session->context->dev) {
		tloge("input invalid param\n");
		return false;
	}

	return true;
}

void teek_close_session(struct teec_session *session)
{
	int32_t ret;
	struct tc_ns_client_context cli_context;
	struct tc_ns_client_login cli_login = {0};

	if (!is_close_sess_param_valid(session))
		return;

	if (session->ops_cnt)
		tloge("session still has commands running\n");
	if (teek_init_context(&cli_context, session->service_id,
		session->session_id, GLOBAL_CMD_ID_CLOSE_SESSION,
		&cli_login) != TEEC_SUCCESS) {
		tloge("init cli context failed just return\n");
		return;
	}
#ifdef CONFIG_AUTH_ENHANCE
	cli_context.teec_token = session->teec_token;
	cli_context.token_len = sizeof(session->teec_token);
#endif
	ret = tc_ns_close_session(session->context->dev, &cli_context);
	if (!ret) {
		session->session_id = 0;
		if (memset_s((uint8_t *)(&session->service_id),
			sizeof(session->service_id), 0x00, UUID_LEN))
			tloge("memset error\n");
#ifdef CONFIG_AUTH_ENHANCE
		if (memset_s(session->teec_token,
			TOKEN_SAVE_LEN, 0x00, TOKEN_SAVE_LEN))
			tloge("memset session's member error\n");
#endif
		session->ops_cnt = 0;
		session->context = NULL;
	} else {
		tloge("close session failed\n");
	}
}

static uint32_t proc_invoke_cmd(struct teec_session *session,
	struct tc_ns_client_context *cli_context, uint32_t *origin)
{
	int32_t ret;
	uint32_t teec_ret;

	ret = tc_ns_send_cmd(session->context->dev, cli_context);
	if (!ret) {
		tlogd("invoke cmd success\n");
		teec_ret = TEEC_SUCCESS;
	} else if (ret < 0) {
		tloge("invoke cmd failed, ioctl errno = %d\n", ret);
		if (ret == -EFAULT)
			teec_ret = TEEC_ERROR_ACCESS_DENIED;
		else if (ret == -ENOMEM)
			teec_ret = TEEC_ERROR_OUT_OF_MEMORY;
		else if (ret == -EINVAL)
			teec_ret = TEEC_ERROR_BAD_PARAMETERS;
		else
			teec_ret = TEEC_ERROR_GENERIC;
		*origin = TEEC_ORIGIN_COMMS;
	} else {
		tloge("invoke cmd failed, code=0x%x, origin=%d\n",
			cli_context->returns.code,
			cli_context->returns.origin);
		teec_ret = (uint32_t)cli_context->returns.code;
		*origin = cli_context->returns.origin;
	}
	return teec_ret;
}

/* This function invokes a Command within the specified Session. */
uint32_t teek_invoke_command(struct teec_session *session, uint32_t cmd_id,
	struct teec_operation *operation, uint32_t *return_origin)
{
	uint32_t teec_ret = TEEC_ERROR_BAD_PARAMETERS;
	uint32_t origin = TEEC_ORIGIN_API;
	struct tc_ns_client_context cli_context;
	struct tc_ns_client_login cli_login = { 0, 0 };

	/* First, check parameters is valid or not */
	if (!session || !session->context) {
		tloge("input invalid session or session->context is null\n");
		goto set_ori;
	}

	teec_ret = teek_check_operation(operation);
	if (teec_ret) {
		tloge("operation is invalid\n");
		goto set_ori;
	}

	if (session->ops_cnt == UINT_MAX) {
		tloge("ops cnt is about to overflow!\n");
		teec_ret = TEEC_ERROR_BAD_PARAMETERS;
		goto set_ori;
	}

	/* Paramters all right, start execution */
	session->ops_cnt++;
	teec_ret = teek_init_context(&cli_context, session->service_id,
		session->session_id, cmd_id, &cli_login);
	if (teec_ret) {
		tloge("init cli context failed\n");
		session->ops_cnt--;
		goto set_ori;
	}

	/* support when operation is null */
	if (operation) {
		cli_context.started = operation->cancel_flag;
		teec_ret = proc_teek_encode(&cli_context, operation);
		tlogv("cli param type %u\n", cli_context.param_types);
		if (teec_ret) {
			session->ops_cnt--;
			goto set_ori;
		}
	}

#ifdef CONFIG_AUTH_ENHANCE
	cli_context.teec_token = session->teec_token;
	cli_context.token_len = sizeof(session->teec_token);
#endif
	teec_ret = proc_invoke_cmd(session, &cli_context, &origin);
	session->ops_cnt--;

set_ori:
	if (teec_ret && return_origin)
		*return_origin = origin;
	return teec_ret;
}

uint32_t teek_send_secfile(struct teec_session *session,
	const char *file_buffer, unsigned int file_size)
{
	if (!file_buffer || !file_size || !session ||
		!session->context || !session->context->dev) {
		tloge("params error!\n");
		return TEEC_ERROR_BAD_PARAMETERS;
	}

	return (uint32_t)tc_ns_load_image_with_lock(session->context->dev,
		file_buffer, file_size);
}
EXPORT_SYMBOL(teek_send_secfile);

/*
 * This function registers a block of existing Client Application memory
 * as a block of Shared Memory within the scope of the specified TEE Context.
 */
uint32_t teek_register_shared_memory(struct teec_context *context,
	struct teec_sharedmemory *sharedmem)
{
	tloge("teek_register_shared_memory not supported\n");
	return TEEC_ERROR_NOT_SUPPORTED;
}

/* begin: for KERNEL-HAL out interface */
int TEEK_IsAgentAlive(unsigned int agent_id)
{
	return teek_is_agent_alive(agent_id);
}
EXPORT_SYMBOL(TEEK_IsAgentAlive);

TEEC_Result TEEK_InitializeContext(const char *name, TEEC_Context *context)
{
	return (TEEC_Result)teek_initialize_context(name,
		(struct teec_context *)context);
}
EXPORT_SYMBOL(TEEK_InitializeContext);

void TEEK_FinalizeContext(TEEC_Context *context)
{
	teek_finalize_context((struct teec_context *)context);
}
EXPORT_SYMBOL(TEEK_FinalizeContext);

/*
 * Function: TEEK_OpenSession
 * Description:   This function opens a new Session
 * Parameters:   context: a pointer to an initialized TEE Context.
 * session: a pointer to a Session structure to open.
 * destination: a pointer to a UUID structure.
 * connectionMethod: the method of connection to use.
 * connectionData: any necessary data required to support the connection method chosen.
 * operation: a pointer to an Operation containing a set of Parameters.
 * returnOrigin: a pointer to a variable which will contain the return origin.
 * Return: TEEC_SUCCESS: success other: failure
 */

TEEC_Result TEEK_OpenSession(TEEC_Context *context, TEEC_Session *session,
	const TEEC_UUID *destination, uint32_t connectionMethod,
	const void *connectionData, TEEC_Operation *operation,
	uint32_t *returnOrigin)
{
	return (TEEC_Result)teek_open_session(
		(struct teec_context *)context, (struct teec_session *)session,
		(struct teec_uuid *)destination, connectionMethod, connectionData,
		(struct teec_operation *)operation, returnOrigin);
}
EXPORT_SYMBOL(TEEK_OpenSession);

void TEEK_CloseSession(TEEC_Session *session)
{
	teek_close_session((struct teec_session *)session);
}
EXPORT_SYMBOL(TEEK_CloseSession);

TEEC_Result TEEK_InvokeCommand(TEEC_Session *session, uint32_t commandID,
	TEEC_Operation *operation, uint32_t *returnOrigin)
{
	return (TEEC_Result)teek_invoke_command(
		(struct teec_session *)session, commandID,
		(struct teec_operation *)operation, returnOrigin);
}
EXPORT_SYMBOL(TEEK_InvokeCommand);

/* end: for KERNEL-HAL out interface */
