/*
 * npu_dfx.c
 *
 * abou npu dfx
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#include "npu_dfx.h"

#include <linux/of.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

#include "npu_resmem.h"
#include "npu_log.h"
#include "npu_platform.h"
#include "npu_platform_register.h"

#define NPU_DFX_CHANNEL_NAME "channel"
#define NPU_DFX_RESMEM_NAME  "buf_idx"

int npu_plat_parse_dfx_desc(struct device_node *module_np,
	struct npu_platform_info *plat_info,
	struct npu_dfx_desc *dfx_desc, u32 idx)
{
	int ret;
	int channel_num;

	channel_num = of_property_count_u32_elems(module_np,
		NPU_DFX_CHANNEL_NAME);
	if (channel_num < 0 || channel_num > NPU_DFX_CHANNEL_MAX_RESOURCE) {
		npu_drv_err("channel_num = %d, out of range\n", channel_num);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(module_np, NPU_DFX_CHANNEL_NAME,
		(u32 *)(dfx_desc->channels), channel_num);
	if (ret < 0) {
		npu_drv_err("parse channels failed\n");
		return -EINVAL;
	}

	dfx_desc->channel_num = (u8)channel_num;
	dfx_desc->bufs = &(plat_info->resmem_info.resmem_desc[idx]);

	npu_drv_debug("dfx_desc: base: 0x%lx, len: %d\n",
		dfx_desc->bufs->base, dfx_desc->bufs->len);

	return 0;
}
