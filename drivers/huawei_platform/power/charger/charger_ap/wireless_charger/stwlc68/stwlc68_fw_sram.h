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

#include <huawei_platform/power/wireless_charger.h>

struct st_fw_update_info {
	/* TX_SRAM or RX_SRAM */
	const enum wireless_mode fw_sram_mode;
	/* SRAM update, from which OTP firmware version */
	const char *name_fw_update_from;
	/* SRAM update, to which OTP firmware version */
	const char *name_fw_update_to;
	/* SRAM */
	const unsigned char *fw_sram;
	const unsigned int fw_sram_size;
	const u16 fw_sram_update_addr;
};

const struct st_fw_update_info st_fw_update[] = {

};

#endif /* _STWLC68_FW_SRAM_H_ */
