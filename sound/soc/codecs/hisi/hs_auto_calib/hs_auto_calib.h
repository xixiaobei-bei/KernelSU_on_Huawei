/*
 * hs_auto_calib.h
 *
 * hs_auto_calib driver
 *
 * Copyright (c) 2016-2020 Huawei Technologies Co., Ltd.
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

#ifndef __HS_AUTO_CALIB_H__
#define __HS_AUTO_CALIB_H__

#include <linux/of.h>

enum adjust_state {
	DEFAULT = 0,
	REC_JUDGE = DEFAULT,
	ERROR_JUDGE,
	VOL_INTERZONE_READJUST,
	BTN_REPORT,
	BTN_RECORD,
	UNEXCEPTION_HANDLE
};

void headset_auto_calib_init(struct device_node *np);
void headset_auto_calib_reset_interzone(void);
void startup_fsm(enum adjust_state state, unsigned int hkadc_value, int *pr_btn_type);

#endif /* __HS_AUTO_CALIB_H__ */

