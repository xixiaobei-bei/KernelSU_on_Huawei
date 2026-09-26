/*
 * security_auth_enhance.h
 *
 * function declaration for token decry, update, verify and so on.
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
#ifndef SECURITY_AUTH_ENHANCE_H
#define SECURITY_AUTH_ENHANCE_H

#include <linux/types.h>
#include "teek_ns_client.h"
#include "smc_smp.h"
#include "gp_ops.h"

#define INC           0x01
#define DEC           0x00
#define UN_SYNCED     0x55
#define IS_SYNCED     0xaa

#ifdef CONFIG_AUTH_ENHANCE

int32_t update_timestamp(const struct tc_ns_smc_cmd *cmd);
int32_t update_chksum(struct tc_ns_smc_cmd *cmd);
int32_t verify_chksum(const struct tc_ns_smc_cmd *cmd);
int32_t sync_timestamp(const struct tc_ns_smc_cmd *cmd, uint8_t *token,
	uint32_t token_len, bool is_global);
int do_encryption(uint8_t *buffer, uint32_t buffer_size,
	uint32_t payload_size, const uint8_t *key);
bool is_opensession_by_index(uint8_t flags, uint32_t cmd_id, int index);
int load_security_enhance_info(const struct tc_call_params *call_params,
	struct tc_op_params *op_params);
int encrypt_login_info(uint32_t login_info_size, uint8_t *buffer,
	const uint8_t *key);
int post_process_token(const struct tc_call_params *call_params,
	struct tc_op_params *op_params);
int append_teec_token(const struct tc_call_params *call_params,
	struct tc_op_params *op_params);
int tzmp2_uid(const struct tc_ns_client_context *client_context,
	struct tc_ns_smc_cmd *smc_cmd, bool is_global);
void clean_session_secure_information(struct tc_ns_session *session);
int get_session_secure_params(struct tc_ns_dev_file *dev_file,
	struct tc_ns_client_context *context, struct tc_ns_session *session);
void free_root_key(void);
int get_session_root_key(void);

#else

static inline int32_t update_timestamp(const struct tc_ns_smc_cmd *cmd)
{
	return 0;
}

static inline int32_t update_chksum(struct tc_ns_smc_cmd *cmd)
{
	return 0;
}

static inline int32_t verify_chksum(const struct tc_ns_smc_cmd *cmd)
{
	return 0;
}

static inline int32_t sync_timestamp(const struct tc_ns_smc_cmd *cmd, uint8_t *token,
	uint32_t token_len, bool is_global)
{
	return 0;
}

static inline int do_encryption(uint8_t *buffer, uint32_t buffer_size,
	uint32_t payload_size, const uint8_t *key)
{
	return 0;
}

static inline bool is_opensession_by_index(uint8_t flags, uint32_t cmd_id, int index)
{
	return false;
}

static inline int load_security_enhance_info(const struct tc_call_params *call_params,
	struct tc_op_params *op_params)
{
	return 0;
}

static inline int encrypt_login_info(uint32_t login_info_size, uint8_t *buffer,
	const uint8_t *key)
{
	return 0;
}

static inline int post_process_token(const struct tc_call_params *call_params,
	struct tc_op_params *op_params)
{
	return 0;
}

static inline int append_teec_token(const struct tc_call_params *call_params,
	struct tc_op_params *op_params)
{
	return 0;
}

static inline int tzmp2_uid(const struct tc_ns_client_context *client_context,
	struct tc_ns_smc_cmd *smc_cmd, bool is_global)
{
	return 0;
}

static inline void clean_session_secure_information(struct tc_ns_session *session)
{
	return;
}

static inline int get_session_secure_params(struct tc_ns_dev_file *dev_file,
	struct tc_ns_client_context *context, struct tc_ns_session *session)
{
	return 0;
}

static inline void free_root_key(void)
{
	return;
}

static inline int get_session_root_key(void)
{
	return 0;
}

#endif

#endif
