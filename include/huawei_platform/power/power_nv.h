/*
 * power_nv.h
 *
 * nv(non-volatile) interface for power module
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

#ifndef _POWER_NV_H_
#define _POWER_NV_H_

#include <linux/types.h>

int power_nv_write(uint32_t nv_number, const char *nv_name,
	const void *data, uint32_t data_len);
int power_nv_read(uint32_t nv_number, const char *nv_name,
	void *data, uint32_t data_len);

#endif /* _POWER_NV_H_ */
