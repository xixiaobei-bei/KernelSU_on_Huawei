/*
 * mtwlc68_fw_sram.h
 *
 * mtwlc68 firmware sram file
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

#ifndef _STWLC68_FW_SRAM_H_
#define _STWLC68_FW_SRAM_H_

#include "stwlc68_fw_sram_data.h"
#include <huawei_platform/power/wireless_charger.h>

#define ST_RAMPATCH_HEADER_SIZE 8
#define ST_RAMPATCH_MAX_SIZE    4112 /* 4k */
#define ST_RAMPATCH_PAGE_SIZE   128

struct st_sram_info {
	const enum wireless_mode fw_sram_mode;
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
};

const struct st_sram_info stwlc68_sram[] = {
	{
		.fw_sram_mode        = WIRELESS_RX_MODE,
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
	},
	{
		.fw_sram_mode        = WIRELESS_RX_MODE,
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
	},
	{
		.fw_sram_mode        = WIRELESS_RX_MODE,
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
	},
	{
		.fw_sram_mode        = WIRELESS_TX_MODE,
		.cut_id_from         = 2,
		.cut_id_to           = 4,
		.cfg_id_from         = 0x3464,
		.cfg_id_to           = 0xFFFF,
		.patch_id_from       = 0x3464,
		.patch_id_to         = 0xFFFF,
		.bad_addr_from       = 0x0000,
		.bad_addr_to         = 0x108c,
		.sram_data           = stwlc68_tx_sram_4603,
		.sram_size           = ARRAY_SIZE(stwlc68_tx_sram_4603),
		.sram_id             = 0x4603,
	},
	{
		.fw_sram_mode        = WIRELESS_TX_MODE,
		.cut_id_from         = 2,
		.cut_id_to           = 4,
		.cfg_id_from         = 0x3464,
		.cfg_id_to           = 0xFFFF,
		.patch_id_from       = 0x3464,
		.patch_id_to         = 0xFFFF,
		.bad_addr_from       = 0x1c18,
		.bad_addr_to         = 0x2000,
		.sram_data           = stwlc68_tx_sram_4603,
		.sram_size           = ARRAY_SIZE(stwlc68_tx_sram_4603),
		.sram_id             = 0x4603,
	},
};

#endif /* _STWLC68_FW_SRAM_H_ */
