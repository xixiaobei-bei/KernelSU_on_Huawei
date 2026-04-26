/*
 * Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef __DP_AUX_H__
#define __DP_AUX_H__

#include <linux/kernel.h>

#define MAX_AUX_RETRY_COUNT 11
#define AUX_RETRY_DELAY_TIME 100
struct dp_ctrl;

/* native read/write 1 byte over aux */
int dptx_read_dpcd(struct dp_ctrl *dptx, uint32_t addr, uint8_t *byte);
int dptx_write_dpcd(struct dp_ctrl *dptx, uint32_t addr, uint8_t byte);
/* native read/write multi-bytes over aux, for DPCD */
int dptx_read_bytes_from_dpcd(struct dp_ctrl *dptx,
	uint32_t reg_addr, uint8_t *bytes, uint32_t len);
int dptx_write_bytes_to_dpcd(struct dp_ctrl *dptx,
	uint32_t reg_addr, uint8_t *bytes, uint32_t len);
/* i2c read/write multi-bytes over aux, for EDID */
int dptx_read_bytes_from_i2c(struct dp_ctrl *dptx,
	uint32_t device_addr, uint8_t *bytes, uint32_t len);
int dptx_write_bytes_to_i2c(struct dp_ctrl *dptx,
	uint32_t device_addr, uint8_t *bytes, uint32_t len);
/* i2c write only address over aux */
int dptx_i2c_address_only(struct dp_ctrl *dptx, uint32_t device_addr);

#endif // DP_AUX_H
