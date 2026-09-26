/*
 * npu_dfx.h
 *
 * abou npu dfx
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#ifndef __NPU_DFX_H
#define __NPU_DFX_H

#include <linux/platform_device.h>
#include "npu_platform.h"

int npu_plat_parse_dfx_desc(struct device_node *module_np,
	struct npu_platform_info *plat_info,
	struct npu_dfx_desc *dfx_desc, u32 idx);

int npu_wait_tscpu_ready_status(void);
#endif
