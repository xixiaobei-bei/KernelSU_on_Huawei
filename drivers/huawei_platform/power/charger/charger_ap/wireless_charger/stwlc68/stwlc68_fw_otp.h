/*
 * mtwlc68_fw_otp.h
 *
 * mtwlc68 firmware otp file
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _STWLC68_FW_OTP_H_
#define _STWLC68_FW_OTP_H_

#include "stwlc68_fw_otp_data.h"

#define STWLC68_RAM_MAX_SIZE            (8 * 1024)

#define STWLC68_OTP_PROGRAM_WR_SIZE     128

#define STWLC68_RAM_FW_START_ADDR       0x00050000
#define STWLC68_RAM_FW_CODE_OFFSET      0x108C
#define STWLC68_RAM_CHECK_VAL1          0xAA /* A: 1010 */
#define STWLC68_RAM_CHECK_VAL2          0x55 /* 5: 0101 */

struct st_fw_otp_info {
	const u8 cut_id_from;
	const u8 cut_id_to;
	const u8 *otp_arr;
	const u16 cfg_id;
	const u32 cfg_size;
	const u16 patch_id;
	const u32 patch_size;
};

const struct st_fw_otp_info st_otp_info[] = {
	{
		.cut_id_from   = 2,
		.cut_id_to     = 10,
		.otp_arr       = stwlc68_otp_data1,
		.cfg_id        = 0x3464,
		.cfg_size      = 512,
		.patch_id      = 0x3464,
		.patch_size    = 14880,
	},
};

#endif /* _STWLC68_FW_OTP_H_ */
