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

struct st_fw_otp_info {
	const u8 target_cut_id;
	const u8 *otp_arr;
	const u16 cfg_id;
	const u32 cfg_size;
	const u16 patch_id;
	const u32 patch_size;
};

const struct st_fw_otp_info st_otp_info[] = {
	{
		.target_cut_id = 2, /* version 1.1 */
		.otp_arr       = stwlc68_otp_data,
		.cfg_id        = 0x23AC,
		.cfg_size      = 512,
		.patch_id      = 0x23AC,
		.patch_size    = 4128,
	},
};

#endif /* _STWLC68_FW_OTP_H_ */
