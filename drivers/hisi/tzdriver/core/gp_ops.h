/*
 * gp_op.h
 *
 * function declaration for alloc global operation and pass params to TEE.
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
#ifndef GP_OPS_H
#define GP_OPS_H
#include "tc_ns_client.h"
#include "teek_ns_client.h"

struct tc_call_params {
	struct tc_ns_dev_file *dev;
	struct tc_ns_client_context *context;
	struct tc_ns_session *sess;
	uint8_t flags;
};

struct tc_op_params {
	struct mb_cmd_pack *mb_pack;
	struct tc_ns_smc_cmd *smc_cmd;
	struct tc_ns_temp_buf local_tmpbuf[TEE_PARAM_NUM];
	uint32_t trans_paramtype[TEE_PARAM_NUM];
	bool op_inited;
};

int write_to_client(void __user *dest, size_t dest_size,
	const void *src, size_t size, uint8_t kernel_api);
int read_from_client(void *dest, size_t dest_size,
	const void __user *src, size_t size, uint8_t kernel_api);
bool tc_user_param_valid(struct tc_ns_client_context *client_context,
	unsigned int index);
int tc_client_call(const struct tc_call_params *call_params);
bool is_tmp_mem(uint32_t param_type);
bool is_ref_mem(uint32_t param_type);
bool is_val_param(uint32_t param_type);
bool is_ion_param(uint32_t param_type);

#endif
