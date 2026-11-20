/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description:  dts and sysfs operation for iscd.
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
#ifndef _HUAWEI_ISCD_INTERFACE_H_
#define _HUAWEI_ISCD_INTERFACE_H_

#include "battery_iscd.h"

void set_fatal_isc_action(struct iscd_info *iscd);
void fatal_isc_protection(struct iscd_info *iscd, unsigned long event);
void iscd_create_sysfs(struct iscd_info *iscd);
void coul_core_get_iscd_info(struct device_node *np, struct iscd_info *iscd);

#endif
