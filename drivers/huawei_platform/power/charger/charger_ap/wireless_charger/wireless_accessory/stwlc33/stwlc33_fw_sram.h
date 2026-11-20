/*
 * stwlc33_fw_sram.h
 *
 * stwlc33 firmware sram file
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#ifndef _STWLC33_FW_SRAM_H_
#define _STWLC33_FW_SRAM_H_

#include <stwlc33_rxfw_sram_1100.h>
#include <stwlc33_txfw_sram_1100.h>
#include <huawei_platform/power/wireless/wireless_charger.h>

struct fw_update_info {
	const enum wireless_mode fw_sram_mode; /* TX_SRAM or RX_SRAM */
	const char *name_fw_update_from; /* from which OTP firmware version */
	const char *name_fw_update_to; /* to which OTP firmware version */
	const unsigned char *fw_sram; /* SRAM */
	const unsigned int fw_sram_size;
	const u16 fw_sram_update_addr;
};

const struct fw_update_info stwlc33_sram[] = {
	{
		.fw_sram_mode        = WIRELESS_RX_MODE,
		.name_fw_update_from = STWLC33_OTP_FW_VERSION_1100H,
		.name_fw_update_to   = STWLC33_OTP_FW_VERSION_1100H_CUT23,
		.fw_sram             = stwlc33_rxfw_sram_1100h,
		.fw_sram_size        = ARRAY_SIZE(stwlc33_rxfw_sram_1100h),
		.fw_sram_update_addr = STWLC33_RX_SRAMUPDATE_ADDR,
	},
	{
		.fw_sram_mode        = WIRELESS_TX_MODE,
		.name_fw_update_from = STWLC33_OTP_FW_VERSION_1100H,
		.name_fw_update_to   = STWLC33_OTP_FW_VERSION_1100H_CUT23,
		.fw_sram             = stwlc33_txfw_sram_1100h,
		.fw_sram_size        = ARRAY_SIZE(stwlc33_txfw_sram_1100h),
		.fw_sram_update_addr = STWLC33_TX_SRAMUPDATE_ADDR,
	},
};

#endif /* _STWLC33_FW_SRAM_H_ */
