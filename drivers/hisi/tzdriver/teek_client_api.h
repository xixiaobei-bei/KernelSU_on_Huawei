/*
 * teek_client_api.h
 *
 * function declaration for libteec interface for kernel CA.
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

#ifndef TEEK_CLIENT_API_H
#define TEEK_CLIENT_API_H
#include "teek_ns_client.h"
#include "teek_client_type.h"

#define TEEC_PARAM_TYPES(param0_type, param1_type, param2_type, param3_type) \
		((param3_type) << 12 | (param2_type) << 8 | \
		 (param1_type) << 4 | (param0_type))

#define TEEC_PARAM_TYPE_GET(param_types, index) \
		(((param_types) >> ((index) << 2)) & 0x0F)

#define TEEC_VALUE_UNDEF 0xFFFFFFFF

#ifdef CONFIG_KERNEL_CLIENT

/*
 * for history reason, we supply two set interface
 * first set is uncapitalized and satisfies kernel code rule
 * second set is capitalized for compatibility
 */
int teek_is_agent_alive(unsigned int agent_id);

uint32_t teek_initialize_context(const char *name,
	struct teec_context *context);

void teek_finalize_context(struct teec_context *context);

uint32_t teek_open_session(struct teec_context *context,
	struct teec_session *session,
	const struct teec_uuid *destination,
	uint32_t connection_method,
	const void *connection_data,
	const struct teec_operation *operation,
	uint32_t *return_origin);

void teek_close_session(struct teec_session *session);

uint32_t teek_send_secfile(struct teec_session *session,
	const char *file_buffer, unsigned int file_size);

uint32_t teek_invoke_command(struct teec_session *session,
	uint32_t cmd_id, struct teec_operation *operation,
	uint32_t *return_origin);

uint32_t teek_register_shared_memory(struct teec_context *context,
	struct teec_sharedmemory *sharedmem);

uint32_t teek_allocate_shared_memory(struct teec_context *context,
	struct teec_sharedmemory *sharedmem);

void teek_release_shared_memory(struct teec_sharedmemory *sharedmem);

void teek_request_cancellation(struct teec_operation *operation);

int TEEK_IsAgentAlive(unsigned int agent_id);

TEEC_Result TEEK_InitializeContext(const char *name, TEEC_Context *context);

void TEEK_FinalizeContext(TEEC_Context *context);

TEEC_Result TEEK_OpenSession(TEEC_Context *context,
	TEEC_Session *session,
	const TEEC_UUID *destination,
	uint32_t connectionMethod,
	const void *connectionData,
	TEEC_Operation *operation,
	uint32_t *returnOrigin);

void TEEK_CloseSession(TEEC_Session *session);

TEEC_Result TEEK_InvokeCommand(TEEC_Session *session,
	uint32_t commandID,
	TEEC_Operation *operation,
	uint32_t *returnOrigin);

#else

static inline int teek_is_agent_alive(unsigned int agent_id)
{
	return TEEC_SUCCESS;
}

static inline int TEEK_IsAgentAlive(unsigned int agent_id)
{
	return TEEC_SUCCESS;
}

static inline uint32_t teek_initialize_context(const char *name,
	struct teec_context *context)
{
	return TEEC_SUCCESS;
}

static inline TEEC_Result TEEK_InitializeContext(const char *name,
	TEEC_Context *context)
{
	return TEEC_SUCCESS;
}

static inline void teek_finalize_context(struct teec_context *context)
{
	(void)context;
}

static inline void TEEK_FinalizeContext(TEEC_Context *context)
{
	(void)context;
}

static inline uint32_t teek_open_session(struct teec_context *context,
	struct teec_session *session,
	const struct teec_uuid *destination,
	uint32_t connection_method,
	const void *connection_data,
	const struct teec_operation *operation,
	uint32_t *return_origin)
{
	return TEEC_SUCCESS;
}

static inline TEEC_Result TEEK_OpenSession(TEEC_Context *context,
	TEEC_Session *session, const TEEC_UUID *destination,
	uint32_t connectionMethod, const void *connectionData,
	TEEC_Operation *operation, uint32_t *returnOrigin)
{
	return TEEC_SUCCESS;
}

static inline void teek_close_session(struct teec_session *session)
{
	(void)session;
}

static inline void TEEK_CloseSession(TEEC_Session *session)
{
	(void)session;
}

static inline uint32_t teek_invoke_command(struct teec_session *session,
	uint32_t cmd_id, struct teec_operation *operation,
	uint32_t *return_origin)
{
	return TEEC_SUCCESS;
}

static inline TEEC_Result TEEK_InvokeCommand(TEEC_Session *session,
	uint32_t commandID, TEEC_Operation *operation, uint32_t *returnOrigin)
{
	return TEEC_SUCCESS;
}

static inline uint32_t teek_send_secfile(struct teec_session *session,
	const char *file_buffer, unsigned int file_size)
{
	return TEEC_SUCCESS;
}

#endif

#endif
