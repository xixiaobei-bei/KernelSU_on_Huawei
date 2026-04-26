/*
 * npu_resmem.h
 *
 * about npu resmem
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
#ifndef __NPU_RESMEM_H
#define __NPU_RESMEM_H

#include <linux/platform_device.h>
#include "npu_platform.h"

int npu_plat_set_resmem(struct npu_platform_info *plat_info);

#endif
