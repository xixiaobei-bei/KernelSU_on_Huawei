/*
 * tvp_adapter.c
 *
 * This is vcodec kernel ca.
 *
 * Copyright (c) 2019-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "tvp_adapter.h"
#include <teek_client_api.h>
#include <linux/mutex.h>
#include "drv_venc_osal.h"

static TEEC_Context g_tee_context;
static TEEC_Session g_tee_session;
static int g_session_count;
static DEFINE_MUTEX(g_tvp_tee_lock);

int32_t init_kernel_ca(void)
{
	TEEC_UUID uuid = {
		0x528822b7, 0xfc78, 0x466b,
		{ 0xb5, 0x7e, 0x62, 0x09, 0x3d, 0x60, 0x34, 0xa7 }
	};
	const char *package_name = "/vendor/bin/hw/android.hardware.media.omx@1.0-service";
	const uint32_t uid = 0;
	TEEC_Operation operation;
	TEEC_Result result;

	mutex_lock(&g_tvp_tee_lock);
	if (g_session_count > 0) {
		HI_INFO_VENC("session has already been opened");
		result = 0;
		goto error;
	}

	(void)memset_s(&operation, sizeof(operation), 0, sizeof(operation));
	(void)memset_s(&g_tee_context, sizeof(g_tee_context), 0, sizeof(g_tee_context));
	(void)memset_s(&g_tee_session, sizeof(g_tee_session), 0, sizeof(g_tee_session));

	result = TEEK_InitializeContext(NULL, &g_tee_context);
	if (result != TEEC_SUCCESS) {
		HI_ERR_VENC("TEEK_InitializeContext Failed");
		result = EAGAIN;
		goto error;
	}

	operation.started = 1;
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE,
						TEEC_NONE,
						TEEC_MEMREF_TEMP_INPUT,
						TEEC_MEMREF_TEMP_INPUT);
	operation.params[2].tmpref.buffer = (void *)&uid;
	operation.params[2].tmpref.size = sizeof(uid);
	operation.params[3].tmpref.buffer = (void *)package_name;
	operation.params[3].tmpref.size = strlen(package_name) + 1;

	result = TEEK_OpenSession(&g_tee_context, &g_tee_session,
		&uuid, TEEC_LOGIN_IDENTIFY, NULL, &operation, NULL);
	if (g_tee_session.context == NULL) {
		HI_ERR_VENC("g_tee_session.context is nullptr");
		TEEK_FinalizeContext(&g_tee_context);
		result = EAGAIN;
		goto error;
	}

	if (result != TEEC_SUCCESS) {
		HI_ERR_VENC("TEEC_OpenSession Failed");
		TEEK_FinalizeContext(&g_tee_context);
		result = EAGAIN;
		goto error;
	}
	g_session_count = 1; // only single session now
	HI_INFO_VENC("TEEK_InitializeContext success");
error:
	mutex_unlock(&g_tvp_tee_lock);
	return result;
}

void deinit_kernel_ca(void)
{
	mutex_lock(&g_tvp_tee_lock);
	if (g_session_count == 0) {
		HI_INFO_VENC("session has been closed");
		mutex_unlock(&g_tvp_tee_lock);
		return;
	}
	TEEK_CloseSession(&g_tee_session);
	TEEK_FinalizeContext(&g_tee_context);
	g_session_count = 0;
	mutex_unlock(&g_tvp_tee_lock);
}

void config_master(enum sec_venc_state state, uint32_t core_id)
{
	TEEC_Result result;
	TEEC_Operation operation;

	(void)memset_s(&operation, sizeof(operation), 0, sizeof(TEEC_Operation));
	mutex_lock(&g_tvp_tee_lock);
	if (g_session_count == 0) {
		mutex_unlock(&g_tvp_tee_lock);
		return;
	}
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
						TEEC_NONE,
						TEEC_NONE,
						TEEC_NONE);
	operation.started = 1;
	operation.params[0].value.a = state;
	operation.params[0].value.b = core_id;
	result = TEEK_InvokeCommand(&g_tee_session, HIVCODEC_CMD_ID_CFG_MASTER,
		&operation, NULL);
	if (result != TEEC_SUCCESS)
		HI_ERR_VENC("TEEC_InvokeCommand HIVCODEC_CMD_ID_CFG_MASTER failed");
	mutex_unlock(&g_tvp_tee_lock);
}
