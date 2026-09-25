/*
 * security_auth_enhance.h
 *
 * function declaration for token decry, update, verify and so on.
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
#ifndef _SECURITY_AUTH_ENHANCE_H_
#define _SECURITY_AUTH_ENHANCE_H_
#include <linux/types.h>
#include "teek_ns_client.h"

#define INC     0x01
#define DEC     0x00
#define UN_SYNCED     0x55
#define IS_SYNCED     0xaa


TEEC_Result update_timestamp(const tc_ns_smc_cmd *cmd);
TEEC_Result update_chksum(tc_ns_smc_cmd *cmd);
TEEC_Result verify_chksum(const tc_ns_smc_cmd *cmd);
TEEC_Result sync_timestamp(const tc_ns_smc_cmd *cmd, uint8_t *token,
	uint32_t token_len, bool global);
#endif
