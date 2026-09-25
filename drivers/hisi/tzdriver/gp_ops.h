/*
 * gp_op.h
 *
 * function declaration for alloc global operation and pass params to TEE.
 *
 * Copyright (c) 2012-2018 Huawei Technologies Co., Ltd.
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
#ifndef _GP_OPS_H_
#define _GP_OPS_H_
#include "tc_ns_client.h"
#include "teek_ns_client.h"

int tc_user_param_valid(tc_ns_client_context *client_context, unsigned int index);
int tc_client_call(tc_ns_client_context *client_context,
	tc_ns_dev_file *dev_file, tc_ns_session *session, uint8_t flags);
#endif
