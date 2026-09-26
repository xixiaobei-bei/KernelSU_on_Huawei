/*
 * gp_buck.h
 *
 * debug for pmic sensor
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
#ifndef _GP_BUCK_H_
#define _GP_BUCK_H_
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/device.h>
#include <linux/debugfs.h>
#include <linux/types.h>

int gp_buck_ctrl_en(u8 pmic_seq, u32 vol, int state);

#endif
