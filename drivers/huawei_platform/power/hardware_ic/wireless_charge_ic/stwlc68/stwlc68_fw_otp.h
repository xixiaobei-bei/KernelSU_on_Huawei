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

#define STWLC68_OTP_START_ADDR          0x60000
#define STWLC68_OTP_CLEAN_ADDR          0x600C0
#define STWLC68_OTP_MAX_SIZE            (16 * 1024)
#define STWLC68_RAM_MAX_SIZE            (8 * 1024)
#define STWLC68_OTP_CMP_SIZE            16
#define STWLC68_OTP_DUMMY_LEN           8

#define STWLC68_OTP_RECOVER_WR_SIZE     4
#define STWLC68_OTP_RECOVER_WR_CNT      4096
#define STWLC68_OTP_PROGRAM_WR_SIZE     128
#define STWLC68_OTP_WR_BLOCK_SIZE       4096

#define STWLC68_RAM_FW_START_ADDR       0x00050000
#define STWLC68_RAM_FW_CODE_OFFSET      0x108C
#define STWLC68_RAM_CHECK_VAL1          0xAA /* A: 1010 */
#define STWLC68_RAM_CHECK_VAL2          0x55 /* 5: 0101 */
#define STWLC68_OTP_FW_START_ADDR       0x00051000
#define STWLC68_OTP_ERR_STATUS_ADDR     0x00068014
#define STWLC68_OTP_ERR_CORRUPT         0x40
#define STWLC68_PGM_IPS_MRR_HI_ADDR     0x0006814C
#define STWLC68_PGM_IPS_MRR_HI_VAL      0x05
#define STWLC68_RD_VERIFY1_MREF_HI_ADDR 0x000680C0
#define STWLC68_RD_VERIFY1_MREF_VAL     0xCD
#define STWLC68_RD_VERIFY_OTP_MREF_ADDR 0x000680C8
#define STWLC68_RD_VERIFY2_OTP_HI_VAL   0xE5
#define STWLC68_SYSTEM_OSC_ADDR         0x2001C169
#define STWLC68_OSC_TRIM_VAL            0x3F
#define STWLC68_ROM_CHECK_ADDR          0x2001C006
#define STWLC68_SYSTEM_CLK_DIV_ADDR     0x2001C00E
#define STWLC68_CLK_DIV_VAL             0x00

/* boot options */
#define STWLC68_BOOT_SET_ADDR           0x2001C218
#define STWLC68_BOOT_FROM_RAM           0x01
#define STWLC68_BOOT_FROM_ROM           0x00

#define STWLC68_RAM_FW_VER_ADDR         0x0006
#define STWLC68_GPIO_STATUS_ADDR        0x0012
#define STWLC68_GPIO_RESET_VAL          0x10
#define STWLC68_SYSTEM_ERR_ADDR         0x001D
#define STWLC68_OTP_WR_CMD_ADDR         0x0145
#define STWLC68_CLEAN_ADDR_UPDATE_VAL   0x40
#define STWLC68_OTP_ENABLE              0x01
#define STWLC68_ROM_CHECK_VAL           0x80
#define STWLC68_FW_CLEAN_ADDR_ADDR      0x0146
#define STWLC68_OTP_WRITE_LENGTH_ADDR   0x014C

#define STWLC68_RAM_ROM_STATUS_UNKNOWN  0x01
#define STWLC68_RAM_ROM_STATUS_OK       0x02
#define STWLC68_RAM_ROM_STATUS_BAD      0x03

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
