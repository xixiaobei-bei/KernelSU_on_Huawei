/*
 * coul_calibration.h
 *
 * calibration for coul
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

#ifndef _COUL_CALIBRATION_H_
#define _COUL_CALIBRATION_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>

struct coul_cali_ops {
	const char *dev_name;
	void *dev_data;
	int (*get_current)(int *, void *);
	int (*get_voltage)(int *, void *);
	int (*get_current_offset)(int *, void *);
	int (*get_current_gain)(unsigned int *, void *);
	int (*get_voltage_offset)(int *, void *);
	int (*get_voltage_gain)(unsigned int *, void *);
	int (*set_current_offset)(int, void *);
	int (*set_current_gain)(unsigned int, void *);
	int (*set_voltage_offset)(int, void *);
	int (*set_voltage_gain)(unsigned int, void *);
	int (*set_cali_mode)(int, void *);
};

enum coul_cali_mode {
	COUL_CALI_MODE_BEGIN = 0,
	COUL_CALI_MODE_MAIN = COUL_CALI_MODE_BEGIN,
	COUL_CALI_MODE_AUX,
	COUL_CALI_MODE_END,
	COUL_CALI_MODE_CALI_ENTER = COUL_CALI_MODE_END,
	COUL_CALI_MODE_CALI_EXIT,
};

enum coul_cali_para {
	COUL_CALI_PARA_BEGIN = 0,
	COUL_CALI_PARA_CUR_A = COUL_CALI_PARA_BEGIN,
	COUL_CALI_PARA_CUR_B,
	COUL_CALI_PARA_VOL_A,
	COUL_CALI_PARA_VOL_B,
	COUL_CALI_PARA_CUR,
	COUL_CALI_PARA_VOL,
	COUL_CALI_PARA_END
};

struct coul_cali_dev {
	struct device *dev;
	unsigned int total_ops;
	int mode;
	struct coul_cali_ops *ops[COUL_CALI_MODE_END];
};

int coul_cali_ops_register(struct coul_cali_ops *ops);
int coul_cali_get_para(int mode, int offset, int *value);

#endif /* _COUL_CALIBRATION_H_ */
