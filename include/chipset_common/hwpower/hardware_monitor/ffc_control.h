/*
 * ffc_control.h
 *
 * ffc control driver
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

#ifndef _FFC_CONTROL_H_
#define _FFC_CONTROL_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/types.h>

#define FFC_MAX_CHARGE_TERM    8

struct ffc_buck_term_para {
	int temp_low;
	int temp_high;
	int vterm_gain;
	int ichg_thre;
	int iterm;
};

enum ffc_buck_term_info {
	FFC_BUCK_TEMP_LOW,
	FFC_BUCK_TEMP_HIGH,
	FFC_BUCK_VTERM_GAIN,
	FFC_BUCK_ICHG_THRE,
	FFC_BUCK_ITERM,
	FFC_BUCK_TERM_TOTAL,
};

struct ffc_dc_vbat_para {
	int vbat_main_th;
	int vbat_aux_th;
};

enum ffc_dc_vbat_info {
	FFC_DC_VBAT_MAIN_TH,
	FFC_DC_VBAT_AUX_TH,
	FFC_DC_VBAT_TOTAL,
};

struct ffc_ctrl_dev {
	struct device *dev;
	struct ffc_buck_term_para buck_term_para[FFC_MAX_CHARGE_TERM];
	struct ffc_dc_vbat_para dc_vbat_para;
	bool buck_term_para_flag;
	bool dc_vbat_para_flag;
};

#ifdef CONFIG_HUAWEI_FFC_CONTROL
int ffc_get_buck_vterm(void);
int ffc_get_buck_iterm(void);
struct ffc_dc_vbat_para *ffc_get_dc_vbat_para(void);
#else
static inline int ffc_get_buck_vterm(void)
{
	return 0;
}

static inline int ffc_get_buck_iterm(void)
{
	return 0;
}

static inline struct ffc_dc_vbat_para *ffc_get_dc_vbat_para(void)
{
	return NULL;
}
#endif /* CONFIG_HUAWEI_FFC_CONTROL */

#endif /* _FFC_CONTROL_H_ */
