/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
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
/*lint -e620*/
#include "hisi_defs.h"
#include "hisi_dp_drv.h"
#include "dp_aux.h"


static int dptx_aux_rw_bytes(struct dp_ctrl *dptx, bool rw, bool i2c, u32 addr, u8 *bytes, u32 len)
{
	int retval;
	u32 i;

	for (i = 0; i < len;) {
		u32 curlen;

		curlen = min_t(u32, len - i, 16);

		if (!i2c) {
			if (dptx->aux_rw) {
				retval = dptx->aux_rw(dptx, rw, i2c, true, false,
					addr + i, &bytes[i], curlen);
			} else {
				retval = -EINVAL;
			}
		} else {
			if (dptx->aux_rw) {
				retval = dptx->aux_rw(dptx, rw, i2c, true, false,
					addr, &bytes[i], curlen);
			} else {
				retval = -EINVAL;
			}
		}

		if (retval) {
			dp_imonitor_set_param_aux_rw(rw, i2c, addr, len, retval);
			return retval;
		}

		i += curlen;
	}

	return 0;
}

int dptx_read_bytes_from_i2c(struct dp_ctrl *dptx, u32 device_addr, u8 *bytes, u32 len)
{
	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	drm_check_and_return((bytes == NULL), -EINVAL, ERR, "[DP] bytes is NULL!\n");

	return dptx_aux_rw_bytes(dptx, true, true, device_addr, bytes, len);
}

int dptx_i2c_address_only(struct dp_ctrl *dptx, u32 device_addr)
{
	u8 bytes[1];

	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");


	if (dptx->aux_rw)
		return dptx->aux_rw(dptx, 0, true, false, true, device_addr, &bytes[0], 1);
	return -EINVAL;
}

int dptx_write_bytes_to_i2c(struct dp_ctrl *dptx, u32 device_addr, u8 *bytes, u32 len)
{
	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	drm_check_and_return((bytes == NULL), -EINVAL, ERR, "[DP] bytes is NULL!\n");

	return dptx_aux_rw_bytes(dptx, false, true, device_addr, bytes, len);
}

int dptx_read_dpcd(struct dp_ctrl *dptx, u32 addr, u8 *byte)
{
	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	drm_check_and_return((byte == NULL), -EINVAL, ERR, "[DP] byte is NULL!\n");

	return dptx_aux_rw_bytes(dptx, true, false, addr, byte, 1);
}

int dptx_write_dpcd(struct dp_ctrl *dptx, u32 addr, u8 byte)
{
	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	return dptx_aux_rw_bytes(dptx, false, false, addr, &byte, 1);
}

int dptx_read_bytes_from_dpcd(struct dp_ctrl *dptx, u32 reg_addr, u8 *bytes, u32 len)
{
	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	drm_check_and_return((bytes == NULL), -EINVAL, ERR, "[DP] bytes is NULL!\n");

	return dptx_aux_rw_bytes(dptx, true, false, reg_addr, bytes, len);
}

int dptx_write_bytes_to_dpcd(struct dp_ctrl *dptx, u32 reg_addr, u8 *bytes, u32 len)
{
	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	drm_check_and_return((bytes == NULL), -EINVAL, ERR, "[DP] bytes is NULL!\n");

	return dptx_aux_rw_bytes(dptx, false, false, reg_addr, bytes, len);
}

/*lint +e620*/
