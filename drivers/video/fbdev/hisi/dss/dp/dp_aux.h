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
 * Copyright (c) 2017 Hisilicon Tech. Co., Ltd. Integrated into the Hisilicon display system.
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
