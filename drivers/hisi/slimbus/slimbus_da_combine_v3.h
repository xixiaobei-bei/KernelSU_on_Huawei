/*
 * slimbus is a kernel driver which is used to manager slimbus devices
 *
 * Copyright (c) 2012-2018 Huawei Technologies Co., Ltd.
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

#ifndef __SLIMBUS_DA_COMBINE_V3_H__
#define __SLIMBUS_DA_COMBINE_V3_H__

#include "slimbus_types.h"

void slimbus_da_combine_v3_callback_register(struct slimbus_device_ops *dev_ops,
	struct slimbus_private_data *pd);

#endif

