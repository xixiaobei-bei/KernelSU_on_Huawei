/*
 * slimbus is a kernel driver which is used to manager slimbus devices
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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

#ifndef __SLIMBUS__PM_H__
#define __SLIMBUS__PM_H__

#include <linux/pm.h>

const struct dev_pm_ops *slimbus_pm_get_ops(void);

#endif /* __SLIMBUS__PM_H__ */

