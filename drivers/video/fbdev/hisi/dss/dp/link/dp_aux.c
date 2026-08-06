/*
 * Copyright (c) 2016 Synopsys, Inc.
 *
 * Synopsys DP TX Linux Software Driver and documentation (hereinafter,
 * "Software") is an Unsupported proprietary work of Synopsys, Inc. unless
 * otherwise expressly agreed to in writing between Synopsys and you.
 *
 * The Software IS NOT an item of Licensed Software or Licensed Product under
 * any End User Software License Agreement or Agreement for Licensed Product
 * with Synopsys or any supplement thereto. You are permitted to use and
 * redistribute this Software in source and binary forms, with or without
 * modification, provided that redistributions of source code must retain this
 * notice. You may not view, use, disclose, copy or distribute this file or
 * any information contained herein except pursuant to this license grant from
 * Synopsys. If you do not agree with this notice, including the disclaimer
 * below, then you are not authorized to use the Software.
 *
 * THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/*
 * Copyright (c) 2020 Hisilicon Tech. Co., Ltd. Integrated into the Hisilicon display system.
 */

#include "dp_aux.h"
#include "hisi_fb.h"
#include "hisi_fb_def.h"
#include "hisi_dp.h"

int dptx_aux_rw_bytes(struct dp_ctrl *dptx,
	bool rw,
	bool i2c,
	uint32_t addr,
	uint8_t *bytes,
	uint32_t len)
{
	int retval;
	uint32_t i;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	dpu_check_and_return((bytes == NULL), -EINVAL, ERR, "[DP] bytes is NULL!\n");

	for (i = 0; i < len; ) {
		uint32_t curlen;

		curlen = min_t(uint32_t, len - i, 16);

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
			huawei_dp_imonitor_set_param_aux_rw(rw, i2c, addr, len, retval);
			return retval;
		}

		i += curlen;
	}

	return 0;
}

int dptx_read_bytes_from_i2c(struct dp_ctrl *dptx,
	uint32_t device_addr,
	uint8_t *bytes,
	uint32_t len)
{
	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	dpu_check_and_return((bytes == NULL), -EINVAL, ERR, "[DP] bytes is NULL!\n");

	return dptx_aux_rw_bytes(dptx, true, true, device_addr, bytes, len);
}

int dptx_i2c_address_only(struct dp_ctrl *dptx, uint32_t device_addr)
{
	uint8_t bytes[1];

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	if (dptx->aux_rw)
		return dptx->aux_rw(dptx, 0, true, false, true, device_addr, &bytes[0], 1);
	return -EINVAL;
}

int dptx_write_bytes_to_i2c(struct dp_ctrl *dptx,
	uint32_t device_addr,
	uint8_t *bytes,
	uint32_t len)
{
	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	dpu_check_and_return((bytes == NULL), -EINVAL, ERR, "[DP] bytes is NULL!\n");

	return dptx_aux_rw_bytes(dptx, false, true, device_addr, bytes, len);
}

int dptx_read_dpcd(struct dp_ctrl *dptx, uint32_t addr, uint8_t *byte)
{
	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	dpu_check_and_return((byte == NULL), -EINVAL, ERR, "[DP] byte is NULL!\n");

	return dptx_aux_rw_bytes(dptx, true, false, addr, byte, 1);
}

int dptx_write_dpcd(struct dp_ctrl *dptx, uint32_t addr, uint8_t byte)
{
	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	return dptx_aux_rw_bytes(dptx, false, false, addr, &byte, 1);
}

int dptx_read_bytes_from_dpcd(struct dp_ctrl *dptx,
	uint32_t reg_addr,
	uint8_t *bytes,
	uint32_t len)
{
	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	dpu_check_and_return((bytes == NULL), -EINVAL, ERR, "[DP] bytes is NULL!\n");

	return dptx_aux_rw_bytes(dptx, true, false, reg_addr, bytes, len);
}

int dptx_write_bytes_to_dpcd(struct dp_ctrl *dptx,
	uint32_t reg_addr,
	uint8_t *bytes,
	uint32_t len)
{
	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	dpu_check_and_return((bytes == NULL), -EINVAL, ERR, "[DP] bytes is NULL!\n");

	return dptx_aux_rw_bytes(dptx, false, false, reg_addr, bytes, len);
}
