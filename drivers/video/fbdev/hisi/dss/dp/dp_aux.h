/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/

#ifndef __DPTX_AUX_H__
#define __DPTX_AUX_H__

#include "../hisi_dp.h"

int __dptx_read_dpcd(struct dp_ctrl *dptx, uint32_t addr, uint8_t *byte);
int __dptx_write_dpcd(struct dp_ctrl *dptx, uint32_t addr, uint8_t byte);

int __dptx_read_bytes_from_dpcd(struct dp_ctrl *dptx,
	uint32_t reg_addr, uint8_t *bytes, uint32_t len);
int __dptx_write_bytes_to_dpcd(struct dp_ctrl *dptx,
	uint32_t reg_addr, uint8_t *bytes, uint32_t len);


int dptx_read_dpcd(struct dp_ctrl *dptx, uint32_t addr, uint8_t *byte);

int dptx_write_dpcd(struct dp_ctrl *dptx, uint32_t addr, uint8_t byte);

static inline int dptx_read_bytes_from_dpcd(struct dp_ctrl *dptx,
	uint32_t reg_addr, uint8_t *bytes, uint32_t len)
{
	return __dptx_read_bytes_from_dpcd(dptx, reg_addr, bytes, len);
}

static inline int dptx_write_bytes_to_dpcd(struct dp_ctrl *dptx,
	uint32_t reg_addr, uint8_t *bytes, uint32_t len)
{
	return __dptx_write_bytes_to_dpcd(dptx, reg_addr, bytes, len);
}

int dptx_aux_rw_bytes(struct dp_ctrl *dptx,
	bool rw, bool i2c, uint32_t addr, uint8_t *bytes, uint32_t len);

int dptx_read_bytes_from_i2c(struct dp_ctrl *dptx,
	uint32_t device_addr, uint8_t *bytes, uint32_t len);

int dptx_write_bytes_to_i2c(struct dp_ctrl *dptx,
	uint32_t device_addr, uint8_t *bytes, uint32_t len);
#endif
