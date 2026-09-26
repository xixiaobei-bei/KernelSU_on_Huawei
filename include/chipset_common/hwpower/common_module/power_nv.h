/*
 * power_nv.h
 *
 * nv(non-volatile) interface for power module
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _POWER_NV_H_
#define _POWER_NV_H_

#include <linux/types.h>

enum power_nv_type {
	POWER_NV_TYPE_BEGIN = 0,
	POWER_NV_BLIMSW = POWER_NV_TYPE_BEGIN,
	POWER_NV_BBINFO,
	POWER_NV_BATHEAT,
	POWER_NV_BATCAP,
	POWER_NV_DEVCOLR,
	POWER_NV_TERMVOL,
	POWER_NV_CHGDIEID,
	POWER_NV_SCCALCUR,
	POWER_NV_CUROFFSET,
	POWER_NV_HWCOUL,
	POWER_NV_TYPE_END,
};

struct power_nv_data_info {
	enum power_nv_type type;
	unsigned int id;
	const char *name;
};

int power_nv_write(enum power_nv_type type, const void *data, uint32_t data_len);
int power_nv_read(enum power_nv_type type, void *data, uint32_t data_len);

#endif /* _POWER_NV_H_ */
