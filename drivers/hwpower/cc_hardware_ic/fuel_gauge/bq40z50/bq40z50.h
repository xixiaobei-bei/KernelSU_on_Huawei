/*
 * bq40z50.h
 *
 * bq40z50 driver
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

#ifndef _BQ40Z50_H_
#define _BQ40Z50_H_

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <asm/unaligned.h>
#include <linux/bitops.h>

/* battery temperature */
#define BQ40Z50_BATT_TEMP_ABNORMAL_LOW       (-20)
#define BQ40Z50_BATT_TEMP_ABNORMAL_HIGH      60
#define BQ40Z50_BATT_TEMP_ZERO               273

/* battery capcaity */
#define BQ40Z50_BATT_CAPACITY_ZERO           0
#define BQ40Z50_BATT_CAPACITY_CRITICAL       5
#define BQ40Z50_BATT_CAPACITY_LOW            15
#define BQ40Z50_BATT_CAPACITY_HIGH           95
#define BQ40Z50_BATT_CAPACITY_FULL           100
#define BQ40Z50_BATT_CAPACITY_DEFAULT        50
#define BQ40Z50_BATT_CAPACITY_LOW_LVL        3
#define BQ40Z50_BATT_CAPACITY_WARNING_LVL    10

/* battery voltage */
#define BQ40Z50_BATT_MAX_VOLTAGE_DEFAULT     13200
#define BQ40Z50_BATT_VOLTAGE_DEFAULT         11460

/* battery status */
#define BQ40Z50_BATT_STATUS_FC               BIT(5)

/* battery brand max len */
#define BQ40Z50_BATT_BRAND_LEN               30

/* charge state */
#define BQ40Z50_CHARGE_STATE_START_CHARGING  1
#define BQ40Z50_CHARGE_STATE_CHRG_DONE       2
#define BQ40Z50_CHARGE_STATE_STOP_CHARGING   3
#define BQ40Z50_CHARGE_STATE_NOT_CHARGING    4

/* bq40z50 reg map */
#define BQ40Z50_REG_TEMP                     0x08 /* temperature */
#define BQ40Z50_REG_VOLT                     0x09 /* voltage */
#define BQ40Z50_REG_CURR                     0x0A /* current */
#define BQ40Z50_REG_RM                       0x0F /* remaining capacity */
#define BQ40Z50_REG_FCC                      0x10 /* full charge capacity */
#define BQ40Z50_REG_AVRGCURR                 0x0B /* average current */
#define BQ40Z50_REG_TTE                      0x12 /* average time to empty */
#define BQ40Z50_REG_TTF                      0x13 /* average time to full */
#define BQ40Z50_REG_CHARGING_CURRENT         0x14 /* charging current */
#define BQ40Z50_REG_CHARGING_VOLTAGE         0x15 /* charging voltage */
#define BQ40Z50_REG_BATTERY_STATUS           0x16 /* battery status */
#define BQ40Z50_REG_CYCLE                    0x17 /* cycle count */
#define BQ40Z50_REG_SOC                      0x0D /* relative state of charge */
#define BQ40Z50_REG_DC                       0x18 /* design capacity */
#define BQ40Z50_REG_SOH                      0x4F /* state of health */
#define BQ40Z50_REG_FLAGS                    0x16 /* battery status */
#define BQ40Z50_REG_MAX_VAL                  0xFFFF /* reg max val */

struct bq40z50_reg_cache {
	s16 temp;
	u16 vol;
	s16 curr;
	s16 avg_curr;
	u16 rm;
	u16 tte;
	u16 ttf;
	u16 fcc;
	u16 dc;
	u16 cycle;
	u16 soc;
	u8 soh;
	u16 flags;
	u16 charge_current;
	u16 charge_voltage;
	u16 status;
};

struct bq40z50_device_info {
	struct i2c_client *client;
	struct device *dev;
	struct bq40z50_reg_cache cache;
	struct mutex rw_lock;
	struct chrg_para_lut *para_batt_data;
	int charge_status;
	int vbat_max;
	u32 is_smart_battery;
};

struct bq40z50_log_data {
	int temp;
	int vbat;
	int ibat;
	int avg_ibat;
	int rm;
	int soc;
	int fcc;
};

#endif /* _BQ40Z50_H_ */
