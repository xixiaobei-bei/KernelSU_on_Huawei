/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: hisi_eis_core_sysfs.h
 *
 * eis core sysfs interface header
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

#ifndef _BATT_EIS_CORE_SYSFS_H_
#define _BATT_EIS_CORE_SYSFS_H_

#include "hisi_eis_core_freq.h"

int eis_create_sysfs_file(struct hisi_eis_device *di);

#endif /* _BATT_EIS_CORE_SYSFS_H_ */

