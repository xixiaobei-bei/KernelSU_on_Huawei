/*
 * lga_check.h
 *
 * lga baoard abnormal monitor driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#ifndef _LGA_CHECK_H_
#define _LGA_CHECK_H_

#define LGA_CK_DELAY_TIME               30000 /* 30s */
#define LGA_CK_ADC_MAX_RETRYS           3
#define LGA_CK_MAX_DMD_REPORT_TIME      5
#define LGA_CK_FRACTURE_FOUND           (-1)
#define LGA_CK_FRACTURE_FREE            0
#define LGA_CK_STATUS_DEFAULT           0

enum lga_ck_mode {
	LGA_CK_MODE_BEGIN = 0,

	LGA_CK_MODE_ADC,

	LGA_CK_MODE_END,
};

enum lga_ck_sysfs_type {
	LGA_CK_SYSFS_TYPE_BEGIN = 0,

	LGA_CK_SYSFS_ADC_VOL,

	LGA_CK_SYSFS_TYPE_END,
};

struct lga_ck_dev {
	struct device *dev;
	struct notifier_block lga_ck_nb;
	struct delayed_work lga_ck_ws;
	u32 lga_ck_mode;
	u32 adc_channel_no;
	int adc_abnormal_thld;
	int adc_vol;
	int lga_ck_abnormal_time;
};

#endif /* _LGA_CHECK_H_ */
