/*
 * stwlc68_fw.h
 *
 * stwlc68 firmware otp, sram file
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _STWLC68_FW_H_
#define _STWLC68_FW_H_

#include "stwlc68_fw_otp_data.h"
#include "stwlc68_fw_sram_data.h"

struct stwlc68_fw_otp_info {
	const u8 cut_id_from;
	const u8 cut_id_to;
	const u8 *otp_arr;
	const u16 cfg_id;
	const u32 cfg_size;
	const u16 patch_id;
	const u32 patch_size;
	int dev_type;
};

const struct stwlc68_fw_otp_info stwlc68_otp_info[] = {
	{
		.cut_id_from   = 2,
		.cut_id_to     = 10,
		.otp_arr       = stwlc68_otp_data1,
		.cfg_id        = 0x3464,
		.cfg_size      = 512,
		.patch_id      = 0x3464,
		.patch_size    = 14880,
		.dev_type      = WIRELESS_DEVICE_UNKNOWN,
	},
	{
		.cut_id_from   = 2,
		.cut_id_to     = 10,
		.otp_arr       = stwlc68_otp_data2,
		.cfg_id        = 0x3464,
		.cfg_size      = 512,
		.patch_id      = 0x3829,
		.patch_size    = 15652,
		.dev_type      = WIRELESS_DEVICE_PAD,
	},
};

struct stwlc68_sram_info {
	const u32 fw_sram_mode;
	u8 cut_id_from;
	u8 cut_id_to;
	u16 cfg_id_from;
	u16 cfg_id_to;
	u16 patch_id_from;
	u16 patch_id_to;
	u16 bad_addr_from;
	u16 bad_addr_to;
	const u8 *sram_data;
	const int sram_size;
	u16 sram_id;
	int dev_type;
};

const struct stwlc68_sram_info stwlc68_sram[] = {
	{
		.fw_sram_mode        = WIRELESS_RX,
		.cut_id_from         = 2,
		.cut_id_to           = 4,
		.cfg_id_from         = 0x3464,
		.cfg_id_to           = 0xFFFF,
		.patch_id_from       = 0x3464,
		.patch_id_to         = 0xFFFF,
		.bad_addr_from       = 0x0000,
		.bad_addr_to         = 0x108c,
		.sram_data           = stwlc68_rx_sram_4709,
		.sram_size           = ARRAY_SIZE(stwlc68_rx_sram_4709),
		.sram_id             = 0x4709,
		.dev_type            = WIRELESS_DEVICE_UNKNOWN,
	},
	{
		.fw_sram_mode        = WIRELESS_RX,
		.cut_id_from         = 4,
		.cut_id_to           = 4,
		.cfg_id_from         = 0x3464,
		.cfg_id_to           = 0xFFFF,
		.patch_id_from       = 0x3464,
		.patch_id_to         = 0xFFFF,
		.bad_addr_from       = 0x11d0,
		.bad_addr_to         = 0x1c17,
		.sram_data           = stwlc68_rx_sram_4504,
		.sram_size           = ARRAY_SIZE(stwlc68_rx_sram_4504),
		.sram_id             = 0x4504,
		.dev_type            = WIRELESS_DEVICE_UNKNOWN,
	},
	{
		.fw_sram_mode        = WIRELESS_RX,
		.cut_id_from         = 2,
		.cut_id_to           = 4,
		.cfg_id_from         = 0x3464,
		.cfg_id_to           = 0xFFFF,
		.patch_id_from       = 0x3464,
		.patch_id_to         = 0xFFFF,
		.bad_addr_from       = 0x1c18,
		.bad_addr_to         = 0x2000,
		.sram_data           = stwlc68_rx_sram_4709,
		.sram_size           = ARRAY_SIZE(stwlc68_rx_sram_4709),
		.sram_id             = 0x4709,
		.dev_type            = WIRELESS_DEVICE_UNKNOWN,
	},
	{
		.fw_sram_mode        = WIRELESS_TX,
		.cut_id_from         = 2,
		.cut_id_to           = 4,
		.cfg_id_from         = 0x3464,
		.cfg_id_to           = 0xFFFF,
		.patch_id_from       = 0x3464,
		.patch_id_to         = 0xFFFF,
		.bad_addr_from       = 0x0000,
		.bad_addr_to         = 0x108c,
		.sram_data           = stwlc68_tx_sram_4752,
		.sram_size           = ARRAY_SIZE(stwlc68_tx_sram_4752),
		.sram_id             = 0x4a34,
		.dev_type            = WIRELESS_DEVICE_UNKNOWN,
	},
	{
		.fw_sram_mode        = WIRELESS_TX,
		.cut_id_from         = 2,
		.cut_id_to           = 4,
		.cfg_id_from         = 0x3464,
		.cfg_id_to           = 0xFFFF,
		.patch_id_from       = 0x3464,
		.patch_id_to         = 0xFFFF,
		.bad_addr_from       = 0x1c18,
		.bad_addr_to         = 0x2000,
		.sram_data           = stwlc68_tx_sram_4752,
		.sram_size           = ARRAY_SIZE(stwlc68_tx_sram_4752),
		.sram_id             = 0x4a34,
		.dev_type            = WIRELESS_DEVICE_UNKNOWN,
	},
	{
		.fw_sram_mode        = WIRELESS_RX,
		.cut_id_from         = 2,
		.cut_id_to           = 4,
		.cfg_id_from         = 0x3464,
		.cfg_id_to           = 0xFFFF,
		.patch_id_from       = 0x3464,
		.patch_id_to         = 0x3828,
		.bad_addr_from       = 0x1c18,
		.bad_addr_to         = 0x2000,
		.sram_data           = stwlc68_rx_sram_4d09,
		.sram_size           = ARRAY_SIZE(stwlc68_rx_sram_4d09),
		.sram_id             = 0x4d09,
		.dev_type            = WIRELESS_DEVICE_PAD,
	},
	{
		.fw_sram_mode        = WIRELESS_RX,
		.cut_id_from         = 2,
		.cut_id_to           = 4,
		.cfg_id_from         = 0x3464,
		.cfg_id_to           = 0xFFFF,
		.patch_id_from       = 0x3829,
		.patch_id_to         = 0xFFFF,
		.bad_addr_from       = 0x1c18,
		.bad_addr_to         = 0x2000,
		.sram_data           = stwlc68_rx_sram_4832,
		.sram_size           = ARRAY_SIZE(stwlc68_rx_sram_4832),
		.sram_id             = 0x4832,
		.dev_type            = WIRELESS_DEVICE_PAD,
	},
	{
		.fw_sram_mode        = WIRELESS_TX,
		.cut_id_from         = 2,
		.cut_id_to           = 4,
		.cfg_id_from         = 0x3464,
		.cfg_id_to           = 0xFFFF,
		.patch_id_from       = 0x3464,
		.patch_id_to         = 0x3828,
		.bad_addr_from       = 0x1c18,
		.bad_addr_to         = 0x2000,
		.sram_data           = stwlc68_tx_sram_4A14,
		.sram_size           = ARRAY_SIZE(stwlc68_tx_sram_4A14),
		.sram_id             = 0x4A14,
		.dev_type            = WIRELESS_DEVICE_PAD,
	},
	{
		.fw_sram_mode        = WIRELESS_TX,
		.cut_id_from         = 2,
		.cut_id_to           = 4,
		.cfg_id_from         = 0x3464,
		.cfg_id_to           = 0xFFFF,
		.patch_id_from       = 0x3829,
		.patch_id_to         = 0xFFFF,
		.bad_addr_from       = 0x1c18,
		.bad_addr_to         = 0x2000,
		.sram_data           = stwlc68_tx_sram_4835,
		.sram_size           = ARRAY_SIZE(stwlc68_tx_sram_4835),
		.sram_id             = 0x4835,
		.dev_type            = WIRELESS_DEVICE_PAD,
	},
};

#endif /* _STWLC68_FW_H_ */
